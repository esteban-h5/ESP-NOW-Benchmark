INSTRUCCIONES PARA EJECUTAR:

	1. Editar var.env con la configuración requerida
	2. Correr main.py para empezar los partidos, se creará el registro de eventos en un json guardado en la carpeta eventos
	
	Graficar:
		1. Abrir data_analisis.ipynb 
		2. Correr primera celda que enlistará todos los eventos guardados
		3. Cambiar variable evento_name por el evento creado y correr demás celdas

	Video:
		1. Correr get_mp4.py
		2. Seleccionar numero de evento creado
		3. Esperar a que programa cree imágenes de cada escena y las convierta en video mp4


Observaciones

	- La posición inicial de los agentes se encuentra en la linea 134-138 de main.py en el constructor del agente. La posición corresponde a lo indicado en la imagen cuadricula.png (Grupo B corresponde a la matriz que comienza desde [0,6] -> [5,8])
	- Para instalar modulos correr 'pip install -r requirements.txt'
	- main.py también admite argumentos para automatizar las pruebas, se despliegan con 'python main.py -h'