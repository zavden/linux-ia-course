# Ejercicio 6.1 — Fundamentos de TCP: Sockets C (`socket`, `bind`, `listen`, `accept`)

## 🎯 Objetivo
Retomar y repasar la magia que usamos en miniserver (bloque05) pero de manera cruda y aislada, dominando cómo los sistemas Operativos exponen el protocolo **TCP/IP** hacia tu Lenguaje C. Aprenderás a crear un Chat Básico.

## 📚 Teoría Mínima
Internet C puro está compuesto de 5 grandes System Calls:
1. `socket(AF_INET, SOCK_STREAM, 0)`: Dame un tubo (File Descriptor) configurado para Familia IPv4 Red (`AF_INET`), en modo ordenado (TCP/`SOCK_STREAM`).
2. `bind(fd, (struct sockaddr*)&addr, sizeof...)`: C Pide al Sistema Operativo adueñarse de un Puerto local `(Ej: Port 9090)`.
3. `listen(fd, 10)`: Prende la antena de OS del Socket. Si te atacan 12 gentes simultáneo, pon 10 en la `cola de espera`.
4. `accept(fd, &addr_de_cliente_remoto, ...)`: Espera (Bloquea tu programa) hasta que suene el teléfono. Retorna un File Descriptor **NUEVO** distinto al original para comunicarte directamente solo con ese cliente!.
5. `read()` y `write()`: Escribes bytes crudos al FD nuevo y viajan a Hong Kong.

## 📝 Instrucciones

Construye `src/main.c`.
1. Inicializa y configura el struct de dirección serv_addr puerto `9090`. (Recuerda usar `htons()` para el puerto e `INADDR_ANY` para escuchar toda tu LAN WiFi Posix interna).
2. Construye tu `socket`, y engánchalo con `bind`, seguido por abrir paso con `listen`.
3. Crea un bucle eterno The Event Loop: `while(1)`.
4. Enfrascáte en el `accept`. (No regresará código hasta que conectes algo). Imprime desde qué IP y Puerto viene usando `inet_ntoa` y `ntohs`.
5. Recíbelo de inmediato con `\nBienvenido a mi Linux C Server Clandestino> \n` usando un `write`.
6. Haz un Loop temporal que use un `read(..)` para ver qué carajo escribe el Intruso y haz Eco (`write` o printar su mensaje x tu consola del master).
7. Cuando Read mande `0` (Lo cerró/desconectó desde putty/telnet), Cierra su socket `close()` y tu Main volverá arriba al Next `accept` a esperar su próximo cliente eterno TCP!.

## ✅ Criterios de Éxito
- Configura este server, abre una SEGUNDA PESTAÑA de terminal Linux local tuya y conéctate usando el comando clásico universal Netcat / Telnet:
  `nc 127.0.0.1 9090`
- Escribe texto y mira cómo fluye la ida y venida cruda C OS, y mira que cuando le das Ctrl+C tu server lo detecta, cerrándole el canal e ignrándolo sin crashear.
