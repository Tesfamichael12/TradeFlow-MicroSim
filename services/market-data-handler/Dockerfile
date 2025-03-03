FROM gcc:9

WORKDIR /app

COPY . .

RUN apt-get update && apt-get install -y cmake

RUN cmake . && make

EXPOSE 8080

CMD ["./market-data-handler"]
