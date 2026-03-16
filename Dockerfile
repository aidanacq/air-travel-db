FROM gcc:14 AS builder
WORKDIR /app
COPY include/ include/
COPY src/ src/
RUN g++ -std=c++17 -O2 -I include src/DataManager.cpp src/main.cpp \
    -o airtraveldb -lpthread -DNOMINMAX

FROM debian:bookworm-slim
WORKDIR /app
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates && rm -rf /var/lib/apt/lists/*
COPY --from=builder /app/airtraveldb .
COPY static/ static/
COPY data/ data/
COPY include/Models.hpp include/Models.hpp
COPY include/DataManager.hpp include/DataManager.hpp
COPY src/DataManager.cpp src/DataManager.cpp
COPY src/main.cpp src/main.cpp
EXPOSE 8080
CMD ["./airtraveldb"]
