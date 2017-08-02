{
  'targets' : [
    {
      'target_name' : 'sqlite',
      'type' : '<(component)',
      'sources' : [
        '<!@(python <(DEPTH)/build/glob_files.py . *.h *.c)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
      },
    'conditions': [
      ['OS == "win"', {
        'msvs_disabled_warnings': [
          4005,
        ],
      }],
    ],
    },
  ]
}
