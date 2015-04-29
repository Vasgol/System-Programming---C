/***************************************** Server Thread functions definitions **********************************************************/


void *Server_thread(void *srvsock);
int Read_Chunk_Request(int sock);
int Write_Chunk_Response(int sock, int chunkpos, FILE *fplog);
