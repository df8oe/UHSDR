FROM gitpod/workspace-full

# add your tools here
ARG GCC_BASE=gcc-arm-none-eabi-8-2019-q3-update
ARG GCC_SHORT=8-2019q3/RC1.1
ARG GCC_URL=https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/${GCC_SHORT}/${GCC_BASE}-linux.tar.bz2
RUN mkdir -p /home/gitpod/.local && wget -q ${GCC_URL} -O - | tar xfj - -C /home/gitpod/.local --strip-components=1
