curl 'http://localhost:8080/search?first_name=G&last_name=S'
curl 'http://localhost:8080/auth' -u 'stephaniecarr:ihwhxUfFyt_pxwSZGHmtzjc@_-YbT@'
curl 'http://localhost:8080/user?id=5'

# получение информации о поездке
curl http://localhost:8081/search?id=1 -u 'stephaniecarr:ihwhxUfFyt_pxwSZGHmtzjc@_-YbT@'

# получение маршрутов пользователя
curl  http://localhost:8082/search -u 'stephaniecarr:ihwhxUfFyt_pxwSZGHmtzjc@_-YbT@'