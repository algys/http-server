FROM ubuntu:16.04

MAINTAINER Algys Ievlev

RUN apt-get -y update --fix-missing
RUN apt install -y cmake
RUN apt install -y g++

USER root

ADD . /opt/http-serv
WORKDIR /opt/http-serv

RUN cmake .
RUN make

CMD ./http-server /etc/httpd.conf

EXPOSE 80