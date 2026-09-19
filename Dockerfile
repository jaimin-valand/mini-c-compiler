FROM gcc:14-bookworm

RUN apt-get update \
    && apt-get install -y --no-install-recommends make bash \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

RUN make test

CMD ["bash"]