{
  'targets' : [
    {
      'target_name' : 'websocketpp',
      'type' : 'none',
      'variables' : {
      },
      'sources' : [
        '<!@(python <(DEPTH)/build/glob_files.py websocketpp *.h *.hpp *.c)',
      ],
      'dependencies' : [
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '.',
        ],
        'msvs_disabled_warnings': [
          4267,
        ],
      },
    },
  ]
}
