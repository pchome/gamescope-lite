dep_xdamage = dependency('xdamage')
dep_xcomposite = dependency('xcomposite')
dep_xcursor = dependency('xcursor')
dep_xrender = dependency('xrender')
dep_xext = dependency('xext')
dep_xfixes = dependency('xfixes')
dep_xxf86vm = dependency('xxf86vm')
dep_xres = dependency('xres')
dep_xmu = dependency('xmu')
dep_xi = dependency('xi')

wayland_server = dependency('wayland-server', version: '>=1.21')
wayland_protos = dependency('wayland-protocols', version: '>=1.17')
xkbcommon = dependency('xkbcommon')
thread_dep = dependency('threads')
cap_dep = dependency('libcap', required: get_option('rt_cap'))
epoll_dep = dependency('epoll-shim', required: false)
sdl3_dep = dependency('SDL3')
avif_dep = dependency('libavif', version: '>=1.0.0', required: get_option('avif_screenshots'))
pixman_dep = dependency('pixman-1')

wlroots_dep = dependency(
  'wlroots-0.18',
  version: ['>= 0.18.0', '< 0.19.0'],
  fallback: 'wlroots',
  default_options: {
    'default_library': 'static',
    'auto_features': 'disabled',
    'examples': false, 'icon_directory': '', 'xcb-errors': 'disabled',
    'xwayland': 'enabled', 'backends': [], 'renderers': [], 'allocators': [],
    'session': 'disabled', 'libliftoff': 'disabled', 'color-management': 'disabled',
    'werror': false,
    'b_lto': get_option('b_lto'),
    'warning_level': get_option('buildtype') == 'debug' ? '1' : '0',
  },
)

glsl_compiler = find_program('glslang', 'glslangValidator', native: true)

# Use --depfile to rebuild shaders when included files have changed. Sadly debian based
# distros don't have up-to-date glslang so we need to check for support first.
if run_command(glsl_compiler, ['--version', '--depfile', 'dummy.dep'], check: false).returncode() == 0
  glsl_generator = generator(
    glsl_compiler,
    output    : ['@BASENAME@.h'],
    arguments : ['-V', '@INPUT@', '--vn', '@BASENAME@', '-o', '@OUTPUT@', '--depfile', '@DEPFILE@', '--quiet'
#                ,'-DHAVE_SOMETHING=@0@'.format(get_option('something').enabled().to_int())
                ],
    depfile   : '@BASENAME@.h.d',
  )
else
  glsl_generator = generator(
    glsl_compiler,
    output    : ['@BASENAME@.h'],
    arguments : ['-V', '@INPUT@', '--vn', '@BASENAME@', '-o', '@OUTPUT@'
#                ,'-DHAVE_SOMETHING=@0@'.format(get_option('something').enabled().to_int())
                ],
  )
endif

shader_src = [
  'shaders/cs_composite_blit.comp',
  'shaders/cs_composite_rcas.comp',
  'shaders/cs_easu.comp',
  'shaders/cs_easu_fp16.comp',
  'shaders/cs_bicubic.comp',
  'shaders/cs_nis.comp',
  'shaders/cs_nis_fp16.comp',
  'shaders/cs_rgb_to_nv12.comp',
]

spirv_shaders = glsl_generator.process(shader_src)

reshade_src = []
reshade_include = include_directories()
reshade_dep = []

if get_option('reshade').enabled()
  reshadefx_version = ['>=6.6.0', '<6.8.0']
  reshadefx_dep = dependency('ReShadeFX', version: reshadefx_version, not_found_message: dep_notfound)
  reshadefx_spirv_dep = dependency('ReShadeFX-spirv', version: reshadefx_version)
  reshadefx_tc_dep = dependency('ReShadeFX-vulkan-type-convert', version: reshadefx_version)
  reshade_dep = [reshadefx_dep, reshadefx_spirv_dep, reshadefx_tc_dep]
endif

imgui_src = []
imgui_include = include_directories()

if get_option('ui').enabled()
imgui_src = [
  '../thirdparty/imgui/imgui.cpp',
#   '../thirdparty/imgui/imgui_demo.cpp',
  '../thirdparty/imgui/imgui_draw.cpp',
  '../thirdparty/imgui/imgui_tables.cpp',
  '../thirdparty/imgui/imgui_widgets.cpp',
  
  '../thirdparty/imgui/backends/imgui_impl_sdl3.cpp',
  '../thirdparty/imgui/backends/imgui_impl_sdlrenderer3.cpp',
]

imgui_include = include_directories(['../thirdparty/imgui','../thirdparty/imgui/backends' ])
endif

required_wlroots_features = ['xwayland']

src = [
  'backend/sdl/sdl_connector.cpp',
  'backend/sdl/sdl_backend.cpp',
  'backend/sdl/sdl_thread.cpp',
  'backend/sdl/sdl_action.cpp',

  'backend/sdl/event/input.cpp',
  'backend/sdl/event/window.cpp',
  'backend/sdl/event/custom.cpp',

  'Utils/TempFiles.cpp',
  'BufferMemo.cpp',
  'steamcompmgr.cpp',
  'commit.cpp',
  'color_helpers.cpp',
  'main.cpp',
  'wlserver.cpp',
  'vblankmanager.cpp',
  'rendervulkan.cpp',
  'mangoapp.cpp',
  'Timeline.cpp',
  'backend.cpp',
  'x11cursor.cpp',
]

if get_option('ui').enabled()
  src += 'backend/sdl/sdl_gui.cpp'
  src += 'backend/sdl/event/ui.cpp'
endif

stb_src = [
  'Utils/stb_image_impl.c',
  'Utils/stb_image_resize_impl.c',
]

if get_option('screenshot').enabled()
  stb_src += 'Utils/stb_image_write_impl.c'
endif

if get_option('headless').enabled()
  src += 'Backends/HeadlessBackend.cpp'
endif

if get_option('reshade').enabled()
  src += 'reshade_effect_manager.cpp'
endif

if get_option('pipewire').enabled()
  src += 'pipewire.cpp'
endif

gamescope_cpp_args = []
gamescope_cpp_args += '-DHAVE_IMGUI=@0@'.format(get_option('ui').enabled().to_int())
gamescope_cpp_args += '-DHAVE_LIBCAP=@0@'.format(get_option('rt_cap').enabled().to_int())
gamescope_cpp_args += '-DHAVE_RESHADE=@0@'.format(get_option('reshade').enabled().to_int())
gamescope_cpp_args += '-DHAVE_HEADLESS=@0@'.format(get_option('headless').enabled().to_int())
gamescope_cpp_args += '-DHAVE_SCREENSHOT=@0@'.format(get_option('screenshot').enabled().to_int())
gamescope_cpp_args += '-DHAVE_AVIF=@0@'.format(get_option('avif_screenshots').enabled().to_int())
gamescope_cpp_args += '-DHAVE_CONVAR=@0@'.format(get_option('convar').enabled().to_int())

src += spirv_shaders
src += protocols_server_src

foreach feat : required_wlroots_features
  if wlroots_dep.get_variable('have_' + feat) != 'true'
    error('Cannot use wlroots built without ' + feat + ' support')
  endif
endforeach

subdir('core')

  executable(
    'gamescope-lite',
    src, gamescope_version, imgui_src, stb_src, reshade_src,
    include_directories : [imgui_include, reshade_include],
    link_with: [core_lib],
    dependencies: [
      dep_x11, dep_xdamage, dep_xcomposite, dep_xrender, dep_xext, dep_xfixes,
      dep_xxf86vm, dep_xres, glm_dep, wayland_server,
      xkbcommon, thread_dep, sdl3_dep, wlroots_dep,
      vulkan_dep, dep_xmu, cap_dep, epoll_dep, pipewire_dep, librt_dep,
      stb_dep, dep_xcursor, avif_dep, dep_xi, reshade_dep,
      pixman_dep,
    ],
    install: true,
    cpp_args: gamescope_cpp_args,
  )


