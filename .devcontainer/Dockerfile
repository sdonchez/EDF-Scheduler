FROM ubuntu:20.04

# non interactive frontend for locales
ENV DEBIAN_FRONTEND=noninteractive

# installing texlive and utils
RUN apt-get update && \
    apt-get -y install --no-install-recommends texlive texlive-latex-extra texlive-extra-utils texlive-bibtex-extra texlive-science biber 

#install smaller packages - for caching
RUN apt-get -y install latexmk make git procps locales curl pandoc pandoc-citeproc xz-utils && \
    curl -L https://github.com/lierdakil/pandoc-crossref/releases/download/v0.3.12.0c/pandoc-crossref-Linux.tar.xz --output ./pandoc-crossref-Linux.tar.xz && tar -xf  ./pandoc-crossref-Linux.tar.xz && rm ./pandoc-crossref-Linux.tar.xz && mv ./pandoc-crossref* /usr/bin &&\
    rm -rf /var/lib/apt/lists/*

# generating locales
RUN sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && \
    dpkg-reconfigure --frontend=noninteractive locales && \
    update-locale LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8 LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8

# installing cpanm & missing latexindent dependencies
RUN curl -L http://cpanmin.us | perl - --self-upgrade && \
    cpanm Log::Dispatch::File YAML::Tiny File::HomeDir