#include &lt; stdio.h & gt;
#include &lt; stdlib.h & gt;
#include &lt; string.h & gt;
#include &lt; signal.h & gt;
#include &lt; unistd.h & gt;
#include &lt; sys / types.h & gt;
#include &lt; sys / socket.h & gt;
#include &lt; netinet / in.h & gt;
#include &lt; arpa / inet.h & gt;
#include &lt; pthread.h & gt;

#include &quot; commons.h & quot; //funciones comunes

#define LENGTH 2048
#define IP_ADDR "127.0.0.1" 
#define PORT 8000

// Global variables
int sockfd = 0;
char name[32];

void reset_stdout(); //vacía buffer de escritura
void emisor();       //maneja los mensajes de entrada
void receptor();     //maneja los mensajes de entrada

int main()
{

    printf("Por favor ingresa tu nombre");
    fgets(name, 32, stdin); //Ingresa el nombre de usuario para el chat
    name[strcspn(name,"\n")] ='\0';
    ;

    if (strlen(name) & gt; 32 || strlen(name) & lt; 2)
    {
        printf(&quot; El nombre debe tener minimo 2 caracteres y maximo 32\n & quot;);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    /* Configuracion del Socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    server_addr.sin_port = htons(PORT);

    // Conectando al servidor
    int err = connect(sockfd, (struct sockaddr *)&amp; server_addr, sizeof(server_addr));
    if (err == -1)
    {
        printf(&quot; ERROR : conectando\n & quot;);
        return EXIT_FAILURE;
    }

    // Send name
    send(sockfd, name, 32, 0);

    printf(&quot; == = Bienvenid @a la sala de chat == =\n & quot;);

    pthread_t hilo_emisor;
    if (pthread_create(&amp; hilo_emisor, NULL, (void *)emisor, NULL) != 0)
    {
        printf(&quot; ERROR : pthread - Hilo del emisor\n & quot;);
        return EXIT_FAILURE;
    }

    pthread_t hilo_receptor;
    if (pthread_create(&amp; hilo_receptor, NULL, (void *)receptor, NULL) != 0)
    {
        printf(&quot; ERROR : pthread - Hilo del receptor\n & quot;);
        return EXIT_FAILURE;
    }
    while (1)
    {
        pause();
    }

    close(sockfd);
    return EXIT_SUCCESS;
}

void emisor()
{

    char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};

    while (1)
    {
        memset(message, 0, LENGTH); //resetea el buffer de
        mensajes
            memset(buffer, 0, LENGTH + 32); //resetea el buffer de
        mensajes
        reset_stdout();

        fgets(message, LENGTH, stdin); //captura el mensaje del
        usuario

            message[strcspn(message, &quot;\n & quot;)] = &#39;
        \0 & #39;
        ;

        sprintf(buffer, &quot; % s - &gt; % s\n & quot;, name, message);

        send(sockfd, buffer, strlen(buffer), 0); //envia el
        mensaje all servidor
    }
}

void receptor()
{
    char message[LENGTH] = {};
    while (1)
    {
        int receive = recv(sockfd, message, LENGTH, 0); //recibe
        los mensajes del servidor

            if (receive & gt; 0)
        {
            printf("%s", message); //Imprime los mensajes del servidor
            reset_stdout();
        }
        else if (receive == 0)
        {
            break;
        }
        memset(message, 0, sizeof(message)); //resetea el buffer
        de mensajes
    }
}