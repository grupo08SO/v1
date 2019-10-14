#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
//programa en C para consultar los datos de la base de datos
//Incluir esta libreria para poder hacer las llamadas en shiva2.upc.es

#include <mysql.h>

int main(int argc, char *argv[])
{
	//Connexio amb la base de dades
	MYSQL *conn;
	int err;
	// Estructura especial para almacenar resultados de consultas 
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	char consulta [80];
	//Creamos una conexion al servidor MYSQL 
	conn = mysql_init(NULL);
	if (conn==NULL) {
		printf ("Error al crear la conexion: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	
	//inicializar la conexion, indicando nuestras claves de acceso
	// al servidor de bases de datos (user,pass)
	conn = mysql_real_connect (conn, "localhost","root", "mysql", NULL, 0, NULL, 0);
	if (conn==NULL)
	{
		printf ("Error al inicializar la conexion: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	//indicamos la base de datos con la que queremos trabajar 
	err=mysql_query(conn, "use juego;");
	if (err!=0)
	{
		printf ("Error al conectar con la base de datos %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	//Creacio del socket
	int sock_conn, sock_listen, ret;
	struct sockaddr_in serv_adr;
	char buff[512];
	char buff2[512];
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	
	
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// escucharemos en el port 9070
	serv_adr.sin_port = htons(9070);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	//La cola de peticiones pendientes no podr? ser superior a 4
	if (listen(sock_listen, 2) < 0)
		printf("Error en el Listen");
	
	int i;
	// Atender las peticiones
	for( ; ; ){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		//sock_conn es el socket que usaremos para este cliente
		
		// Ahora recibimos su nombre, que dejamos en buff
		ret=read(sock_conn,buff, sizeof(buff));
		printf ("Recibido\n");
		
		// Tenemos que a?adirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		buff[ret]='\0';
		char *p =strtok(buff,"/");
		int codigo =  atoi (p);
		
		if (codigo ==1) //logueo
	    {
			char nombre[20];
			p=strtok(NULL,"/");
			printf(nombre,p);
			char contra[20];
			p=strtok(NULL,"/");
			printf(contra,p);
			char query [150];
			sprintf(query,"SELECT jugador.usuario, jugador.contraseña FROM jugador WHERE jugador.usuario='%s' AND jugador.contraseña='%s'",nombre, contra);
			err=mysql_query (conn,query);
			resultado = mysql_store_result (conn);
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				sprintf(buff2, "no");
				close(sock_conn);
			}
			else sprintf (buff2, "SI");
			printf ("%s\n", buff2);
			// Y lo enviamos
			write (sock_conn,buff2, strlen(buff2));
			close(sock_conn);
			
		}
			
		if (codigo==2) //Quiere saber los puntos totales
		{
			char nombre[20];
			p=strtok(NULL,"/");
			printf(nombre,p);
			char query [150];
			sprintf(query,"SELECT SUM(relacion.puntuacion) FROM jugador, partidas, relacion WHERE jugador.usuario ='%s' AND jugador.id = relacion.idjug AND relacion.idpartida = partidas.id",nombre);
			printf("Consulta: %s\n",query);
			err=mysql_query(conn,query);
			resultado = mysql_store_result (conn);
		    if (err!=0) {
			printf ("Error al consultar datos de la base %u %s\n",
					mysql_errno(conn), mysql_error(conn));
			sprintf(buff2,"f");
			write(sock_conn,buff2,strlen(buff2));
			close(sock_conn);
			}
			else 
			     row = mysql_fetch_row (resultado);
				 sprintf(buff2,row[0]);
				 write(sock_conn,buff2,strlen(buff2));
				 close(sock_conn);
				 
			
		}
		if (codigo==3)//Quiere saber el tiempo jugado
		{
			char nombre[20];
            p=strtok(NULL,"/");
			char query [150];
			sprintf(query,"SELECT SUM(partidas.duracion) FROM jugador, partidas, relacion WHERE jugador.usuario = '%s' AND partidas.id = relacion.idpartida AND relacion.idpartida = jugador.id", nombre);
			err=mysql_query(conn,query);
			resultado = mysql_store_result (conn);
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				sprintf(buff2, "f");
				write(sock_conn,buff2,strlen(buff2));
				close(sock_conn);
			}
			else 
			row = mysql_fetch_row (resultado);
			sprintf(buff2,row[0]);
			write(sock_conn,buff2,strlen(buff2));
			close(sock_conn);
		}
		if (codigo==4)
		{
			char nombre1[20];
			p=strtok(NULL,"/");
			char nombre2[20];
			char *p2 =strtok(NULL,"/");
			char query [150];
			sprintf (query, "SELECT COUNT(partidas.ganador) FROM partidas WHERE partidas.ganador = '%s' AND partidas.participantes = '%s-%s'",nombre1,nombre1,nombre2);
			err=mysql_query(conn,query);
			resultado = mysql_store_result (conn);
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				sprintf(buff2, "f");
				write(sock_conn,buff2,strlen(buff2));
				close(sock_conn);
			}
			else 
		    row = mysql_fetch_row (resultado);
			sprintf(buff2,row[0]);
			write(sock_conn,buff2,strlen(buff2));
			close(sock_conn);
			
			
			
		}
		
			
		
		// Se acabo el servicio para este cliente
			close(sock_conn); 
	}
}
