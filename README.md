# stdroller

# to build
    make

# to run
    some_cmd | -f ./stdroller some_file_name
    
# currently supported options

usage:
  stdroller -f file_name|--file=file_name [--sufix|-s] [-r|--rollover] [--limit|-l] [--help|-h]

options:

  -f file_name
  --file=file_name    - File name where log will be stored

  -s
  --sufix             - Should I create suffix files or not?
                        If you want me to create suffix files
                        I will create each new suffix file after limit
                        is reached. Otherwise, I will overwrite
                        oryginal log. I mean, I will overwrite it
                        like destroying it, cleaning, you will get
                        no nothing.

  -l limit
  --limit=limit       - Log size limit; default is 1GB. Unfortunately
                        you have to specify full size, like 1000000000 for 1GB

  -r
  --rollover          - If you set roll over flag, I will not destroy content
                        However, tail -F will not work in this case. So, you
                        are the one to decide which log style you prefer

  -h
  --help              - surprize, surprize! I will show you help message.
