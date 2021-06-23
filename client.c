#include <sys/socket>
#include <netinet/in>
#include  arpa/inet
#include  stdio
#include  stdlib
#include  unistd
#include  errno
#include  string
#include  pthread
#include  sys/types
#include commons

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define IP_ADDR &quot;127.0.0.1&quot;
#define PORT 8000

int cli_count = 0;
static int uid = 10;

/* Tipo de dato del cliente*/
typedef struct{
struct sockaddr_in address;
int sock;
int uid;
char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

void queue_add(client_t *cl);
void queue_remove(int uid);
void send_message(char *s, int uid);
void *handle_client(void *arg);

int main(){
int option = 1;
int listenfd = 0, connfd = 0;

struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;
pthread_t tid;

/* Configuracion del Socket */
listenfd = socket(AF_INET, SOCK_STREAM, 0);
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
serv_addr.sin_port = htons(PORT);

if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT |
SO_REUSEADDR), (char *)&amp;option, sizeof(option))   0){
DieWithError(&quot;ERROR: setsockopt falló&quot;);
}

/* Bind */
if (bind(listenfd, (struct sockaddr *)&amp;serv_addr,
sizeof(serv_addr))   0){
DieWithError(&quot;ERROR: El enlace de socket falló&quot;);
}

/* Listen */
if (listen(listenfd, 10)   0){
DieWithError(&quot;ERROR: Fallo al abrir el socket&quot;);
}

printf(&quot;=== Sala de chat iniciada ===\n&quot;);

while (1){
socklen_t clilen = sizeof(cli_addr);
connfd = accept(listenfd, (struct sockaddr *)&amp;cli_addr,
&amp;clilen);

/* Comprueba el maximo de clientes */
if ((cli_count + 1) == MAX_CLIENTS){
printf(&quot;Max clients reached. Rejected: &quot;);
printf(&quot;:%d\n&quot;, cli_addr.sin_port);
close(connfd);
continue;
}

/* Configuracion del cliente aceptado */
client_t *cli = (client_t *)malloc(sizeof(client_t));
cli-&gt;address = cli_addr;
cli-&gt;sock = connfd;
cli-&gt;uid = uid++;

/* Agrega el cliente a la cola y crea el hilo del cliente
*/
queue_add(cli);
pthread_create(&amp;tid, NULL, &amp;handle_client, (void *)cli);

/* Reduce uso del CPU */
sleep(1);

}

return EXIT_SUCCESS;
}

/* agrega clientes a la cola */
void queue_add(client_t *cl){
for (int i = 0; i   MAX_CLIENTS; ++i) {
if (!clients[i]) {
clients[i] = cl;
break;
}
}
}

/* Elimina clientes de la cola */
void queue_remove(int uid){
for (int i = 0; i   MAX_CLIENTS; ++i) {
if (clients[i]) {
if (clients[i]-&gt;uid == uid) {
clients[i] = NULL;
break;
}
}
}
}

/* envia mensajes a los clientes */
void send_message(char *s, int uid){
for (int i = 0; i   MAX_CLIENTS; ++i){
if (clients[i]){
if (clients[i]-&gt;uid != uid) {
if (send(clients[i]-&gt;sock, s, strlen(s), 0)   0){
DieWithError(&quot;ERROR: error al enviar el
mensaje&quot;);
}
}
}
}
}

/* Maneja todas las comunicaciones del cliente */
void *handle_client(void *arg){
char buff_out[BUFFER_SZ];
char name[32];
int leave_flag = 0;

cli_count++;
client_t *cli = (client_t *)arg;

memset(buff_out, 0, BUFFER_SZ); //resetea el buffer de
mensajes

//obtiene y comprueba el nombre del usuario

if (recv(cli-&gt;sock, name, 32, 0)  = 0 || strlen(name)   2 ||
strlen(name) &gt;= 32 - 1){
printf(&quot;No ingreso un nombre.\n&quot;);
leave_flag = 1;
}
else{
strcpy(cli-&gt;name, name);
name[strcspn(name, &quot;\n&quot;)] = &#39;\0&#39;;

sprintf(buff_out, &quot;%s se ha unido - %d conectados\n&quot;,
name, cli_count);
printf(&quot;%s&quot;, buff_out);
send_message(buff_out, cli-&gt;uid);
}

while (1){
if (leave_flag){
break;
}
memset(buff_out, 0, BUFFER_SZ); //resetea el buffer de
mensajes
int receive = recv(cli-&gt;sock, buff_out, BUFFER_SZ, 0);
if (receive &gt; 0){
if (strlen(buff_out) &gt; 0){
send_message(buff_out, cli-&gt;uid);
printf(&quot;%s\n&quot;, buff_out);
}

}
else if (receive == 0){
cli_count--;
sprintf(buff_out, &quot;%s se ha ido - %d conectados\n&quot;,
cli-&gt;name, cli_count);
printf(&quot;%s&quot;, buff_out);
send_message(buff_out, cli-&gt;uid);
leave_flag = 1;
}
else{
printf(&quot;ERROR: -1\n&quot;);
leave_flag = 1;
}

}

/* Elimina el cliente de la cola y el socket junto con el hilo
de ejecución */
close(cli-&gt;sock);
queue_remove(cli-&gt;uid);
free(cli);
pthread_detach(pthread_self());

return NULL;