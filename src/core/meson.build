compiler_name = cppc.get_id()
compiler_version = cppc.version()

vcs_tag_cmd = ['git', 'describe', '--always', '--tags', '--dirty=+']
vcs_tag_str = run_command(vcs_tag_cmd, check: false).stdout().strip()
version_tag = vcs_tag_str + ' (' + compiler_name + ' ' + compiler_version + ')'

version_conf = configuration_data()
version_conf.set('PROJECT_NAME', meson.project_name())
version_conf.set('VCS_TAG', version_tag)
version_conf.set('BUILDTYPE', get_option('buildtype'))
version_conf.set('PREFIX', get_option('prefix'))
version_conf.set('DATADIR', get_option('datadir'))
version_conf.set('LIBDIR', get_option('libdir'))
version_conf.set('BINDIR', get_option('bindir'))

version_conf.set('HAVE_PIPEWIRE', get_option('pipewire').enabled().to_string().to_lower())
version_conf.set('HAVE_LIBCAP', get_option('rt_cap').enabled().to_string().to_lower())
version_conf.set('HAVE_RESHADE', get_option('reshade').enabled().to_string().to_lower())
version_conf.set('HAVE_SCREENSHOT', get_option('screenshot').enabled().to_string().to_lower())
version_conf.set('HAVE_HEADLESS', get_option('headless').enabled().to_string().to_lower())
version_conf.set('HAVE_AVIF', get_option('avif_screenshots').enabled().to_string().to_lower())
version_conf.set('HAVE_STEAM', get_option('steam').enabled().to_string().to_lower())
# version_conf.set('HAVE_MANGOAPP', get_option('mangoapp').enabled().to_string().to_lower())
version_conf.set('HAVE_IMGUI', get_option('ui').enabled().to_string().to_lower())

gamescope_version = configure_file(
  input : 'version.hpp.in',
  output: 'version.hpp',
  configuration : version_conf
)

gamescope_core_src = files([
  'convar.cpp',
  'log.cpp',
  'process.cpp',
  'version.cpp',
])

core_lib = static_library('core', gamescope_core_src, gamescope_version, install: false)

executable('gamescopereaper-lite', 'gamescopereaper.cpp', link_with: [core_lib], dependencies: [cap_dep], install:true)
