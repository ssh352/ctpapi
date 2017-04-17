{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'folw_bin_reproduce',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py . *.h *.cc)',
      ],
      'dependencies' : [
        '../follow_strategy_mode/follow_strategy_mode.gyp:follow_strategy_mode',
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
    },
  ]
}
