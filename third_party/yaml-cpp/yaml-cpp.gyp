{
  'targets' : [
    {
      'target_name' : 'yaml-cpp',
      'type' : '<(component)',
      'sources' : [
        '<!@(python <(DEPTH)/build/glob_files.py include *.h)',
        '<!@(python <(DEPTH)/build/glob_files.py src *.h *.cpp)',
      ],
      'include_dirs': [
        'include'
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'include',
        ],
      },
    },
  ]
}
