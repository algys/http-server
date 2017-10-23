FROM ubuntu:16.04

MAINTAINER Algys Ievlev

RUN apt-get -y update --fix-missing
RUN apt-get -y upgrade
RUN apt install -y cmake
RUN apt install -y g++

USER root

ADD . /opt/http-serv
WORKDIR /opt/http-serv

RUN cmake
RUN cmake .
RUN make

CMD ./http-server conf.txt

EXPOSE 80