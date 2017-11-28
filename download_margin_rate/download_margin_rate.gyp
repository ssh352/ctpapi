{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'download_margin_rate',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py . *.h *.cc)',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
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
