FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    manpages-dev \
    vim \
    nano \
    iputils-ping \
    inetutils-traceroute \
    iproute2 \
    openssh-server \
    sudo \
    curl telnet dnsutils vim 

RUN useradd -m student
RUN echo "student:password" | chpasswd
RUN echo "root:password" | chpasswd
RUN adduser student sudo
RUN usermod -aG sudo student
RUN echo "student ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers

RUN mkdir /run/sshd \
    && chown root:22 /run/sshd

USER student
RUN mkdir /home/student/lab-work

RUN sudo sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
EXPOSE 22
EXPOSE 3000
WORKDIR /home/student/lab-work
CMD sudo /usr/sbin/sshd -D && /bin/bash