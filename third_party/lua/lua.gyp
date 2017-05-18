{
  'targets' : [
    {
      'target_name' : 'lua53',
      'type' : 'static_library',
      'variables' : {
      },
      'sources' : [
        '<!@(python <(DEPTH)/build/glob_files.py src *.h *.hpp *.c)',
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
          'src',
        ],
      },
    },
  ]
}
