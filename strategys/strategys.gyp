{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'strategys',
      'type' : 'none',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py . *.h *.cc)',
      ],
#      'sources/' :[
#        ['exclude','main.cc'],
#        ['exclude', 'unittest/.*'],
#      ],
      'dependencies' : [
      ],
    },
  ]
}
