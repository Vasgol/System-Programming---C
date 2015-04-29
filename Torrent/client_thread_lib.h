/***************************************** Client Thread functions definitions **********************************************************/


void *Client_thread();
int Write_Chunk_Request(int sock, int chunkpos);
int Read_Chunk_Response(int sock, char* description, char* value, int chunkpos, FILE *fplog, PeerList *plistrec);
