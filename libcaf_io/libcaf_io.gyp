{
  'targets' : [
    {
      'target_name' : 'libcaf_io',
      'type' : 'static_library',
      'variables' : {
        'win_characterset%': 2,
      },
      'sources' : [
       '<!@(python ../../../build/glob_files.py caf src *.hpp)',
      ],
      'dependencies' : [
        '../libcaf_core/libcaf_core.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '.',
      ],
      'conditions': [
        ['OS == "win" and target_arch=="x64"', {
          'msvs_disabled_warnings': [
            4267,
          ],
        }]
      ]
    },
  ]
}
