{
  'targets' : [
  {
    'target_name' : 'libcaf_core',
    'type' : 'static_library',
    'variables' : {
    },
    'sources' : [
       '<!@(python ../../../build/glob_files.py caf src *.hpp)',
    ],
    'dependencies' : [
      'libcaf_build_config',
    ],
    'defines' : [
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '.',
    ],

    'all_dependent_settings': {
      'include_dirs': [
        '.',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'iphlpapi.lib',
              ],
            },
          },
        }],
      ],
    },
    'conditions': [
      ['OS == "win"', {
        # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
        'msvs_disabled_warnings': [
          4297,
        ],
      }],
      ['OS == "win" and target_arch=="x64"', {
        'msvs_disabled_warnings': [
          4267,
        ],
      }]
    ],
  },
  {
    'target_name': 'libcaf_build_config',
    'type': 'none',
    'hard_dependency': 1,
    'variables': {
      'log_level%': '--log-level=-1',
      'use_asio%': '',
      'no_exceptions%': '',
      'enable_runtime_checks%': '',
      'no_mem_management%': '',

      'variables': {
        'caf_log_level%': '-1',
        'caf_use_asio%': '0',
        'caf_no_exceptions%': '0',
        'caf_enable_runtime_checks%': '0',
        'caf_no_mem_management%': '0',
      },
      'conditions': [
        ['caf_log_level!=-1',{
          'log_level': '--log-level=<(caf_log_level)',
        }],
        ['caf_use_asio==1', {
          'use_asio': '--use-asio',
        }],
        ['caf_no_exceptions==1', {
          'no_exceptions': '--no-exceptions',
        }],
        ['caf_enable_runtime_checks==1', {
          'enable_runtime_checks': '--enable-runtime-checks',
        }],
        ['caf_no_mem_management==1', {
          'no_mem_management': '--no-mem-management',
        }],
      ],
    },
    'direct_dependent_settings': {
      'include_dirs': [
        '<(SHARED_INTERMEDIATE_DIR)',
      ],
    },
    'actions': [
      {
        'action_name': 'gen_libcaf_build_config',
        'inputs': [
          '../cmake/build_config.hpp.in',
        ],
        'outputs': [
          '<(SHARED_INTERMEDIATE_DIR)/caf/detail/build_config.hpp',
        ],
        'action': [
          'python',
          'libcaf_build_config_gen.py',
          '<@(_inputs)',
          '<@(_outputs)',
          '<(log_level)',
          '<(use_asio)',
          '<(enable_runtime_checks)',
          '<(no_mem_management)',
          '<(no_exceptions)',
        ],
        'message': 'Generating libcaf build_config header file: <@(_outputs)',
      },
    ],
  }
  ]
}
