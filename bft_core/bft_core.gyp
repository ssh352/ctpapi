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

    ],
    'defines' : [
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '..',
      '<(DEPTH)/third_party/actor-framework/libcaf_core'
    ],
  },
  ]
}
