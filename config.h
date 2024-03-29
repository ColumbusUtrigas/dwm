/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>
#include "tcl.c"

/* appearance */
static const unsigned int borderpx  = 0;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft = 0;   	/* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     /* 0 means no systray */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "JetBrainsMono Nerd Font:antialias=true:autohint=true", };
static const char dmenufont[]       = "monospace:size=10:antialias=true:autohint=true";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#003355";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

static const char *keyboardLayouts[] = { "gb", "ru" };
static int currentKeyboardLayout = 0;
static void switchKeyboardLayout(const Arg *arg) {
	currentKeyboardLayout = (currentKeyboardLayout + arg->i) % LENGTH(keyboardLayouts);
	const char *switchcmd[] = { "xkb-switch", "-s", keyboardLayouts[currentKeyboardLayout], NULL };
	const char *reloadcmd[] = { "pkill", "-RTMIN+11", "dwmblocks", NULL };
	const Arg sarg = {.v = switchcmd};
	spawn(&sarg);
	const Arg rarg = {.v = reloadcmd};
	spawn(&rarg);
}

static const char *const autostart[] = {
	"dwmblocks", NULL,
	"setxkbmap", "-layout", "gb,ru", NULL,
	"bg-set-image", NULL,
	"nm-applet", NULL,
	"blueman-applet", NULL,
	"optimus-manager-qt", NULL,
	"sh", "-c",  "CM_SELECTIONS=\"clipboard\" CM_OWN_CLIPBOARD=1 clipmenud", NULL,
	"/usr/lib/polkit-gnome/polkit-gnome-authentication-agent-1", NULL,
//	"picom", NULL,
	NULL /* terminate */
};


/* tagging */
static const char *tags[] = { "", "", "", "", "", "", "", "", "" };

static const unsigned int ulinepad     = 5;    /* horizontal padding between the underline and tag */
static const unsigned int ulinestroke  = 2;    /* thickness / height of the underline */
static const unsigned int ulinevoffset = 0;    /* how far above the bottom of the bar the line should appear */
static const int ulineall              = 0;    /* 1 to show underline on all tags, 0 for just the active ones */

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask  isfloating isterminal noswallow monitor */
	{ "Gimp",       NULL,       NULL,       0,         1,         0,         0,       -1 },
	{ "Firefox",    NULL,       NULL,       1 << 8,    0,         0,         0,       -1 },
	{ "Galculator", NULL,       NULL,       0,         1,         0,         0,       -1 },
	{ "Alacritty",  NULL,       NULL,       0,         0,         1,         0,       -1 },
	{ "ranger",     NULL,       NULL,       0,         0,         1,         0,       -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "|||",      tcl  },    /* three-column layout */
	{ "[M]",      monocle },
	{ "><>",      NULL },    /* no layout function means floating behavior */
};

/* key definitions */
#define MODKEY Mod1Mask
#define WINKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
#define STATUSBAR "dwmblocks"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "alacritty", NULL };
static const char *browsercmd[] = { "firefox", NULL };
static const char* screenshot[] = { "flameshot", "gui", NULL };
static const char* lockscreencmd[] = { "slock", NULL };
static const char* clipboardcmd[] = { "clipmenu", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ControlMask,           XK_t,      spawn,          {.v = termcmd } },
	{ MODKEY|ControlMask,           XK_f,      spawn,          {.v = browsercmd } },
	{ 0, XF86XK_AudioRaiseVolume,              spawn,          SHCMD("pamixer --allow-boost -i 5; pkill -RTMIN+10 dwmblocks") },
	{ 0, XF86XK_AudioLowerVolume,              spawn,          SHCMD("pamixer --allow-boost -d 5; pkill -RTMIN+10 dwmblocks") },
	{ 0, XF86XK_AudioMute,                     spawn,          SHCMD("pamixer -t; pkill -RTMIN+10 dwmblocks") },
	{ 0, XK_Print,                             spawn,          {.v = screenshot } },
	{ WINKEY,                       XK_space,  switchKeyboardLayout, {.i = +1 } },
	{ WINKEY,                       XK_l,      spawn,          {.v = lockscreencmd } },
	{ WINKEY,                       XK_v,      spawn,          {.v = clipboardcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button1,        sigstatusbar,          {.i = 1 } },
	{ ClkStatusText,        0,              Button2,        sigstatusbar,          {.i = 2 } },
	{ ClkStatusText,        0,              Button3,        sigstatusbar,          {.i = 3 } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

