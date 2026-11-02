#include "android_port.h"
#include "runtime.h"
#include "touch_input.h"
#include "host_overlay.h"
#include "recomp_runtime_ui.h"
#include "pixel_presenter.h"
#include "zelda_icons.h"
#include "game_photo_album.h"
#include "rom_regions.h"
#include "runtime_bus_bridge.h"
#include "gba_bus.h"
#include <jni.h>
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <string>
#include <vector>
#include <unistd.h>

int MinishMain(int, char**);
namespace {
using namespace gbarecomp;
constexpr auto space = OverlaySpace::Drawable;
float scale = 1.0f;
int opacity = 180;
std::uint16_t pressed = 0;
bool menu_was_down = false;
bool stock_pad_visible = false;
void java_action(const char* name) {
    auto* env=static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    auto activity=static_cast<jobject>(SDL_AndroidGetActivity());
    if(env && activity) {
        auto cls=env->GetObjectClass(activity);
        auto method=env->GetMethodID(cls,name,"()V");
        if(method) env->CallVoidMethod(activity,method);
        env->DeleteLocalRef(cls);env->DeleteLocalRef(activity);
    }
}
void capture_frame(const std::uint8_t* rgb,int w,int h) {
    if(w<1 || h<1 || w>1024 || h>1024) return;
    auto* env=static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    auto activity=static_cast<jobject>(SDL_AndroidGetActivity());
    if(!env || !activity) return;
    auto cls=env->GetObjectClass(activity);
    auto method=env->GetMethodID(cls,"captureFrame","([BIIIIII)V");
    jbyteArray data=env->NewByteArray(w*h*3);
    if(method && data) {
        env->SetByteArrayRegion(data,0,w*h*3,reinterpret_cast<const jbyte*>(rgb));
        int area=-1,room=-1,x=-1,y=-1;
        if(auto* bus=gbarecomp::active_bus();!minish::is_europe() && bus && bus->iwram_ptr()[0x1002]==2) {
            const auto* i=bus->iwram_ptr();const auto* e=bus->ewram_ptr();
            const auto* location=i+0xBF0;
            if(i[0x1004]==7) location=e+0x32EDC; // UI backup while a subtask owns RAM.
            area=location[4];room=location[5];
            if(i[0x1004]!=7) { x=i[0x118E]|(i[0x118F]<<8);y=i[0x1192]|(i[0x1193]<<8); }
        }
        env->CallVoidMethod(activity,method,data,w,h,area,room,x,y);
    }
    if(data) env->DeleteLocalRef(data);
    env->DeleteLocalRef(cls);env->DeleteLocalRef(activity);
}
void close_runtime_menu() {
    SDL_Event event{};event.type=SDL_CONTROLLERBUTTONDOWN;
    event.cbutton.button=SDL_CONTROLLER_BUTTON_GUIDE;
    SDL_PushEvent(&event);event.type=SDL_CONTROLLERBUTTONUP;SDL_PushEvent(&event);
}
int menu_action(const char* key) {
    if(!key) return 0;
    if(std::string(key)=="minish.album") { close_runtime_menu();java_action("showAlbum");return 1; }
    if(std::string(key)!="minish.library") return 0;
    // Guide closes the entire menu; Back may only leave a selected section.
    // The paused presentation loop must close before it can consume quit.
    close_runtime_menu();SDL_Event event{};
    event.type=SDL_QUIT; SDL_PushEvent(&event);
    return 1;
}
const char* const pixel_choices[]={"Pixel perfeito", "Suave (Scale2x)"};
int menu_get(const char* key, int* value) {
    if (!key || !value || std::string(key)!="minish.pixels") return 0;
    *value=static_cast<int>(minish::pixel_mode()); return 1;
}
int menu_set(const char* key, int value) {
    if (!key || std::string(key)!="minish.pixels" || value<0 || value>1) return 0;
    minish::set_pixel_mode(static_cast<minish::PixelMode>(value));
    auto* env=static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    auto activity=static_cast<jobject>(SDL_AndroidGetActivity());
    if (env && activity) {
        jclass cls=env->GetObjectClass(activity);
        jmethodID save=env->GetMethodID(cls,"persistPixelMode","(I)V");
        if (save) env->CallVoidMethod(activity,save,value);
        env->DeleteLocalRef(cls); env->DeleteLocalRef(activity);
    }
    return 1;
}
const RecompRuntimeUiItem menu_items[]{ {
    "minish.album", "Album", "Gerenciar fotos e anotacoes",
    "Galeria Android. A pagina de camera no inventario requer a ROM USA.",
    RECOMP_RUNTIME_UI_ACTION,0,0,0,nullptr,0,nullptr
}, {
    "minish.pixels", "Graphics", "Estilo dos pixels",
    "Bordas suaves ou pixels originais. Ajuste salvo automaticamente.",
    RECOMP_RUNTIME_UI_CHOICE,0,1,1,pixel_choices,2,nullptr
}, {
    "minish.library", "System", "Voltar para o inicio",
    "Encerra a partida e volta para a biblioteca. O save do cartucho e preservado.",
    RECOMP_RUNTIME_UI_ACTION,0,0,0,nullptr,0,nullptr
} };
struct Button { float x, y, w, h; int bit; const char* label; bool round; };
struct Layout {
    float u, dx, dy;
    std::vector<Button> buttons;
};
Layout layout(float w, float h, int left, int top, int right, int bottom,
              const PresentationLayout& game={}) {
    const float u = std::min(h / 5.5f, w / 12.0f) * scale;
    const float pad = std::max(12.f, u * .12f);
    const float x0 = left + pad, x1 = w - right - pad;
    const float y0 = top + pad, y1 = h - bottom - pad;
    Layout l{u, x0 + u, y1 - u, {}};
    const float r = u * .46f;
    l.buttons = {
        {x1-r, y1-u*1.65f, r*2, r*2, 0, "A", true},
        {x1-u*1.55f, y1-r, r*2, r*2, 1, "B", true},
        {w*.44f, y1-u*.24f, u*.95f, u*.42f, 2, "SELECT", false},
        {w*.56f, y1-u*.24f, u*.95f, u*.42f, 3, "START", false},
        {x1-u*.65f, y0+u*.27f, u*1.3f, u*.53f, 8, "R", false},
        {x0+u*.65f, y0+u*.27f, u*1.3f, u*.53f, 9, "L", false},
        {w*.5f, y0+u*.24f, u*1.05f, u*.48f, -1, "MENU", false}
    };
    return l;
}
bool contains(const Button& b, float x, float y) {
    if (b.round) return std::hypot(x-b.x,y-b.y) <= b.w*.56f;
    return std::abs(x-b.x) <= b.w*.56f && std::abs(y-b.y) <= b.h*.65f;
}
std::uint16_t input(const TouchFrameInfo* f) {
    pressed = 0;
    stock_pad_visible = f && f->pad_visible;
    if (!f) { menu_was_down=false;return 0x3ff; }
    minish::photo_album_frame(f->frame_count);
    auto l = layout(f->drawable_width, f->drawable_height, f->safe_insets.left,
        f->safe_insets.top, f->safe_insets.right, f->safe_insets.bottom,f->game_rect);
    bool menu=false;
    for (std::size_t i=0; i<f->point_count; ++i) {
        const auto& p=f->points[i];
        float x=p.drawable_x-l.dx, y=p.drawable_y-l.dy;
        // Sliding between directions and diagonals works without lifting.
        if (!stock_pad_visible && std::abs(x)<l.u && std::abs(y)<l.u) {
            const float dead=l.u*.16f;
            if (x>dead && std::abs(y)<std::abs(x)*2.5f) pressed|=1<<4;
            if (x<-dead && std::abs(y)<std::abs(x)*2.5f) pressed|=1<<5;
            if (y<-dead && std::abs(x)<std::abs(y)*2.5f) pressed|=1<<6;
            if (y>dead && std::abs(x)<std::abs(y)*2.5f) pressed|=1<<7;
        }
        for (const auto& b:l.buttons) if (contains(b,p.drawable_x,p.drawable_y)) {
            if(b.bit>=0) { if(!stock_pad_visible) pressed |= 1<<b.bit; }
            else if(b.bit==-1 && !stock_pad_visible) menu=true;
        }
    }
    if (menu && !menu_was_down) {
        SDL_Event e{}; e.type=SDL_KEYDOWN; e.key.keysym.sym=SDLK_AC_BACK;
        SDL_PushEvent(&e);
        e.type=SDL_KEYUP; SDL_PushEvent(&e);
    }
    menu_was_down=menu;
    return 0x3ff & ~pressed;
}
OverlayColor gold{225,199,119,220}, ink{8,33,27,180}, light{209,242,217,255};
void draw(HostOverlay* o) {
    if (!o || TouchHub::instance().presentation().drawable_width<=0) return;
    // The engine's optional stock pad takes over when its toggle is enabled.
    // The last frame's input mask is presentation feedback only.
    auto l=layout(o->drawable_width(),o->drawable_height(),o->safe_insets().left,
        o->safe_insets().top,o->safe_insets().right,o->safe_insets().bottom,o->game_rect());
    auto bg=ink; bg.a=opacity;
    const float u=l.u, t=std::max(2.f,u*.012f);
    if(!stock_pad_visible) {
    o->fill_circle(l.dx,l.dy,u*.98f,{8,23,20,(std::uint8_t)(opacity*.55f)},space);
    o->fill_rect(l.dx-u*.32f,l.dy-u*.89f,u*.64f,u*1.78f,u*.09f,bg,space);
    o->fill_rect(l.dx-u*.89f,l.dy-u*.32f,u*1.78f,u*.64f,u*.09f,bg,space);
    const float xs[]={.65f,-.65f,0,0}, ys[]={0,0,-.65f,.65f};
    for(int i=0;i<4;++i) {
        auto c=(pressed&(1<<(i+4)))?light:gold;
        const float x=l.dx+xs[i]*u,y=l.dy+ys[i]*u;
        const float vx=xs[i]==0?1:0,vy=ys[i]==0?1:0;
        o->line(x-xs[i]*u*.13f+vx*u*.1f,y-ys[i]*u*.13f+vy*u*.1f,x,y,t*2,c,space);
        o->line(x,y,x-xs[i]*u*.13f-vx*u*.1f,y-ys[i]*u*.13f-vy*u*.1f,t*2,c,space);
    }
    o->stroke_circle(l.dx,l.dy,u*.18f,t,gold,space);
    minish::draw_icon(o,minish::Icon::Crest,l.dx,l.dy,u*.24f,space);
    }
    for(const auto& b:l.buttons) {
        if(stock_pad_visible && b.bit>=-1) continue;
        const bool down=b.bit>=0 && (pressed&(1<<b.bit));
        auto c=down?OverlayColor{57,116,77,(std::uint8_t)opacity}:bg;
        if(b.round) {
            o->fill_circle(b.x,b.y,b.w*.5f,c,space);
            o->stroke_circle(b.x,b.y,b.w*.5f,t,gold,space);
            o->stroke_circle(b.x,b.y,b.w*.43f,t*.5f,{146,173,113,120},space);
        } else {
            o->fill_rect(b.x-b.w/2,b.y-b.h/2,b.w,b.h,b.h*.3f,c,space);
            o->stroke_rect(b.x-b.w/2,b.y-b.h/2,b.w,b.h,b.h*.3f,t,gold,space);
        }
        using minish::Icon;
        Icon icon=Icon::Crest;
        switch(b.bit) {
        case 0:icon=Icon::Sword;break;case 1:icon=Icon::Shield;break;
        case 2:icon=Icon::Map;break;case 3:icon=Icon::Pouch;break;
        case 8:icon=Icon::Cap;break;case 9:icon=Icon::Feather;break;
        case -2:icon=Icon::Camera;break;case -3:icon=Icon::Album;break;
        }
        const float size=b.round?u*.7f:b.h*.5f;
        minish::draw_icon(o,icon,b.round?b.x:b.x-b.w*.3f,b.y,size,space);
        if(b.round) {
            o->fill_circle(b.x+b.w*.32f,b.y+b.h*.3f,u*.15f,{15,49,34,245},space);
            o->text(b.x+b.w*.32f,b.y+b.h*.3f-u*.11f,u*.22f,light,b.label,OverlayAlign::Center,space);
        } else {
            o->text(b.x+b.w*.18f,b.y-u*.06f,u*.12f,light,b.label,OverlayAlign::Center,space);
        }
    }
}
}
namespace minish {
void configure_android(gbarecomp::RunOptions& opts) {
    minish::set_capture_sink(&capture_frame);
    if (!minish::is_europe()) minish::configure_game_photo_album();
    else minish::set_frame_filter(nullptr);
    opts.input_frame=&input; opts.host_overlay=&draw;
    opts.touch_pad_default=0;
    opts.touch_gesture_claims=gbarecomp::kTouchClaimLongPress;
    opts.ui_touch_friendly=true;
    opts.resume_suspend_state_on_launch=true;
    opts.expose_assist_tools=true;
    opts.save_state_slot_count=10;
    opts.diagnostic_replay_default=false;
    opts.ui_extra_items=menu_items;
    opts.ui_extra_item_count=3;
    opts.ui_get=&menu_get;
    opts.ui_set=&menu_set;
    opts.ui_action=&menu_action;
}
}
extern "C" __attribute__((visibility("default"))) int SDL_main(int argc,char** argv) {
    SDL_SetMainReady();
    const char* root=SDL_AndroidGetInternalStoragePath();
    if(!root || chdir(root)) return 2;
    std::filesystem::create_directories("saves");
    // Persist the runtime's existing diagnostics, including strict failures,
    // without depending on Android's discarded native stdout/stderr streams.
    std::filesystem::create_directories("diagnostics");
    int log=open("diagnostics/native.log",O_WRONLY|O_CREAT|O_APPEND|O_TRUNC,0600);
    if(log>=0) { dup2(log,STDOUT_FILENO); dup2(log,STDERR_FILENO); close(log); }
    setvbuf(stdout,nullptr,_IOLBF,0);
    unsetenv("GBARECOMP_STRICT_STATIC");
    unsetenv("SDL_AUDIODRIVER");
    minish::set_pixel_mode(minish::PixelMode::Smooth);
    std::vector<char*> args;
    bool europe=false;
    for(int i=0;i<argc;++i) {
        if(std::string(argv[i])=="--minish-region" && i+1<argc) {
            const std::string region=argv[++i];
            if(region!="usa" && region!="eu") return 2;
            europe=region=="eu";
        } else if(std::string(argv[i])=="--touch-size" && i+1<argc) {
            scale=std::clamp(std::atoi(argv[++i])/100.f,.8f,1.3f);
        } else if(std::string(argv[i])=="--touch-opacity" && i+1<argc) {
            opacity=std::clamp(std::atoi(argv[++i])*255/100,65,240);
        } else if(std::string(argv[i])=="--pixel-mode" && i+1<argc) {
            minish::set_pixel_mode(std::string(argv[++i])=="0"?
                minish::PixelMode::Perfect:minish::PixelMode::Smooth);
        } else if(std::string(argv[i])=="--qa-muted") {
            SDL_setenv("SDL_AUDIODRIVER","dummy",1);
        } else if(std::string(argv[i])=="--qa-strict") {
            SDL_setenv("GBARECOMP_STRICT_STATIC","1",1);
        } else args.push_back(argv[i]);
    }
    minish::select_europe(europe);
    int count=args.size(); args.push_back(nullptr);
    return MinishMain(count,args.data());
}
