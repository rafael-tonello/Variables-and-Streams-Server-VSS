FROM ubuntu:22.04

RUN apt update
RUN apt install -y clang llvm make libssl-dev

COPY ./compileTests.sh /usr/bin/compileTests.sh
CMD ["echo", "use compileTests.sh to compile tests"]
