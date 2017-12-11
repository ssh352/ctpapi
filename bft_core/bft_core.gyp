{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
  {
    'target_name' : 'bft_core',
    'type' : 'shared_library',
    'variables' : {
    },
    'sources' : [
      '<!@(python ../build/glob_files.py . *.h *.cc)',
    ],
    'sources/' :[
      ['exclude', 'unittest/.*'],
    ],
    'dependencies' : [
      '<(DEPTH)/third_party/actor-framework/libcaf_core/libcaf_core.gyp:libcaf_core',
    ],
    'defines' : [
      'CORE_IMPLEMENTATION',
    ],
    'export_dependent_settings': [
      '<(DEPTH)/third_party/actor-framework/libcaf_core/libcaf_core.gyp:libcaf_core',
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '..',
      # '<(DEPTH)/third_party/actor-framework/libcaf_core'
    ],
  },
  ]
}
