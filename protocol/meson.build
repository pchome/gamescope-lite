fs = import('fs')

wayland_scanner_dep = dependency('wayland-scanner', native: true)
wayland_scanner_path = wayland_scanner_dep.get_variable(pkgconfig: 'wayland_scanner')
wayland_scanner = find_program(wayland_scanner_path, native: true)

wayland_protos = dependency('wayland-protocols',
	fallback: 'wayland-protocols',
	default_options: ['tests=false'],
)
wl_protocol_dir = wayland_protos.get_variable('pkgdatadir')

protocols = [
	# Upstream protocols
	wl_protocol_dir / 'stable/xdg-shell/xdg-shell.xml',
	wl_protocol_dir / 'stable/presentation-time/presentation-time.xml',
	wl_protocol_dir / 'unstable/pointer-constraints/pointer-constraints-unstable-v1.xml',
	wl_protocol_dir / 'staging/linux-drm-syncobj/linux-drm-syncobj-v1.xml',

	# Gamescope protocols
	'gamescope-xwayland.xml',

	'gamescope-input-method.xml',
	'gamescope-control.xml',

	'gamescope-swapchain.xml',
	'gamescope-private.xml',
	'gamescope-action-binding.xml',

	# wlroots protocols
	'wlr-layer-shell-unstable-v1.xml',
]

if get_option('pipewire').enabled()
  protocols += 'gamescope-pipewire.xml'
endif

if get_option('reshade').enabled()
  protocols += 'gamescope-reshade.xml'
endif

protocols_server_src = []

foreach xml : protocols
	name = fs.name(xml).replace('.xml', '')

	code = custom_target(
		name + '-protocol.c',
		input: xml,
		output: '@BASENAME@-protocol.c',
		command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
	)

	server_header = custom_target(
		name + '-protocol.h',
		input: xml,
		output: '@BASENAME@-protocol.h',
		command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
	)

	protocols_server_src += [code, server_header]
endforeach
