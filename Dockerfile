FROM alpine:latest AS build

ARG YAML_CPP_VERSION="0.6.3"

RUN apk --update upgrade && \
        apk add git cmake g++ make boost-dev boost-static && \
        wget "https://github.com/jbeder/yaml-cpp/archive/yaml-cpp-$YAML_CPP_VERSION.tar.gz" -O /tmp/yaml-cpp.tar.gz && \
        cd /tmp && tar xf ./yaml-cpp.tar.gz && \
        cd ./yaml-cpp-yaml-cpp-$YAML_CPP_VERSION && \
        mkdir -p ./build && \
        cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && \
        make -j 4 && make install

COPY . /tmp/cm

RUN cd /tmp/cm && \
        cmake -B build && \
        cd build && \
        make -j 2 && \
        strip ./cm

FROM alpine:latest

COPY --from=build /tmp/cm/build/cm /usr/bin/cm

ENTRYPOINT [ "/usr/bin/cm" ]
CMD [ "-j", "/etc/cm.yaml" ]
