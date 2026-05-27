project(
  'gamescope-lite',
  'c',
  'cpp',
  meson_version: '>=1.2.0',
  default_options: {
    'cpp_std'           : 'c++23',
    'c_std'             : 'c23,c11',
    'warning_level'     : '2',
    'b_ndebug'          : 'if-release',
    'force_fallback_for': 'wlroots',
    'wrap_mode'         : 'nodownload',
    
  },
)

fallbacks = get_option('force_fallback_for')
if not fallbacks.contains('wlroots')
  error('"force_fallback_for=wlroots" is missing!',
    'Please do not remove entries from "force_fallback_for" if you are packaging the project.',
    'We pull in these projects at specific commits/forks/builds for a reason.',
    'If you are not packaging, remove this line to continue.')
endif

add_project_arguments([
  '-DWLR_USE_UNSTABLE',
], language: 'cpp')

cppc = meson.get_compiler('cpp')

data_dir = get_option('datadir')
prefix = get_option('prefix')
lib_dir = get_option('libdir')

add_project_arguments(cppc.get_supported_arguments([
  '-Wno-unused-parameter',
  '-Wno-missing-field-initializers',
  '-Wno-c99-designator',
  '-Wno-invalid-offsetof',
  '-Wno-unused-const-variable',
  '-Wno-volatile', # glm warning
  '-Wno-deprecated-volatile',
  '-Wno-ignored-qualifiers', # reshade warning
  '-Wno-missing-braces',
]), language: 'cpp')

# # Disable "unused" type warnings for 'wlroots' when assertions disabled by NDEBUG
# if get_option('buildtype') == 'plain' or get_option('buildtype') == 'release'
#   add_project_arguments(cppc.get_supported_arguments([
#     '-Wno-unused-variable',
#     '-Wno-unused-but-set-variable',
#   ]), language: ['c', 'cpp'])
# endif

# These warnings indicate likely runtime problems with LTO, so promote them to errors.
# If a package fails to build with these, LTO should not be used there.
if get_option('b_lto')
  add_project_arguments(cppc.get_supported_arguments([
    '-Werror=odr',
    '-Werror=lto-type-mismatch',
    '-Werror=strict-aliasing',
  ]), language: ['c','cpp'])
endif

add_project_arguments(cppc.get_supported_arguments([
  '-fno-exceptions',
]), language: 'cpp')

pipewire_dep = dependency('libpipewire-0.3', required: get_option('pipewire'))
librt_dep = cppc.find_library('rt', required : get_option('pipewire'))

dep_x11 = dependency('x11')
vulkan_dep = dependency('vulkan')

# INFO: WrapDB: `meson wrap update-db`
dep_notfound = 'Install system package or use "--wrap-mode=default" option.'
glm_dep = dependency('glm', not_found_message: dep_notfound)
stb_dep = dependency('stb', not_found_message: dep_notfound)

# Vulkan headers are installed separately from the loader (which ships the
# pkg-config file)
if not cppc.check_header('vulkan/vulkan.h', dependencies: vulkan_dep)
  error('Missing vulkan-headers')
endif

subdir('protocol')
subdir('src')

