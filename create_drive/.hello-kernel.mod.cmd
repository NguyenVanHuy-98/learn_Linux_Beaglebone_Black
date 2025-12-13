cmd_/home/huy/worlk/create_drive/hello-kernel.mod := printf '%s\n'   hello-kernel.o | awk '!x[$$0]++ { print("/home/huy/worlk/create_drive/"$$0) }' > /home/huy/worlk/create_drive/hello-kernel.mod
