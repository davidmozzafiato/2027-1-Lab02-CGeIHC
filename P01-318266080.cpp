#include <stdio.h>
#include <string.h>
#include <glew.h>
#include <glfw3.h>
#include <time.h>   // Necesario para time()
#include <stdlib.h> // Necesario para srand() y rand()

// Dimensiones de la ventana
const int WIDTH = 800, HEIGHT = 800;
GLuint VAO, VBO, shader;

// LENGUAJE DE SHADER (GLSL)

// Vertex Shader
static const char* vShader = "						\n\
#version 330										\n\
layout (location = 0) in vec3 pos;					\n\
void main()											\n\
{													\n\
	gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f); 	\n\
}";

// Fragment Shader (Color blanco para contrastar con rojo, verde y azul)
static const char* fShader = "						\n\
#version 330										\n\
out vec4 color;										\n\
void main()											\n\
{													\n\
	color = vec4(1.0f, 1.0f, 1.0f, 1.0f); 			\n\
}";

void CrearFiguras()
{
	GLfloat vertices[] = {
		// ==========================================
		// LETRA 'D' (24 vértices - 8 triángulos)
		// ==========================================
		// 1. Barra vertical izquierda
		-0.75f, -0.45f, 0.0f,  -0.65f, -0.45f, 0.0f,  -0.65f,  0.45f, 0.0f,
		-0.75f, -0.45f, 0.0f,  -0.65f,  0.45f, 0.0f,  -0.75f,  0.45f, 0.0f,

		// 2. Barra horizontal superior
		-0.65f,  0.35f, 0.0f,  -0.35f,  0.35f, 0.0f,  -0.35f,  0.45f, 0.0f,
		-0.65f,  0.35f, 0.0f,  -0.35f,  0.45f, 0.0f,  -0.65f,  0.45f, 0.0f,

		// 3. Barra vertical derecha
		-0.35f, -0.35f, 0.0f,  -0.25f, -0.35f, 0.0f,  -0.25f,  0.35f, 0.0f,
		-0.35f, -0.35f, 0.0f,  -0.25f,  0.35f, 0.0f,  -0.35f,  0.35f, 0.0f,

		// 4. Barra horizontal inferior
		-0.65f, -0.45f, 0.0f,  -0.35f, -0.45f, 0.0f,  -0.35f, -0.35f, 0.0f,
		-0.65f, -0.45f, 0.0f,  -0.35f, -0.35f, 0.0f,  -0.65f, -0.35f, 0.0f,

		// ==========================================
		// LETRA 'P' (24 vértices - 8 triángulos)
		// ==========================================
		// 1. Barra vertical izquierda
		-0.15f, -0.45f, 0.0f,  -0.05f, -0.45f, 0.0f,  -0.05f,  0.45f, 0.0f,
		-0.15f, -0.45f, 0.0f,  -0.05f,  0.45f, 0.0f,  -0.15f,  0.45f, 0.0f,

		// 2. Barra horizontal superior
		-0.05f,  0.35f, 0.0f,   0.25f,  0.35f, 0.0f,   0.25f,  0.45f, 0.0f,
		-0.05f,  0.35f, 0.0f,   0.25f,  0.45f, 0.0f,  -0.05f,  0.45f, 0.0f,

		// 3. Barra vertical derecha superior
		 0.15f,  0.00f, 0.0f,   0.25f,  0.00f, 0.0f,   0.25f,  0.35f, 0.0f,
		 0.15f,  0.00f, 0.0f,   0.25f,  0.35f, 0.0f,   0.15f,  0.35f, 0.0f,

		 // 4. Barra horizontal media
		 -0.05f,  0.00f, 0.0f,   0.25f,  0.00f, 0.0f,   0.25f,  0.10f, 0.0f,
		 -0.05f,  0.00f, 0.0f,   0.25f,  0.10f, 0.0f,  -0.05f,  0.10f, 0.0f,

		 // ==========================================
		 // LETRA 'S' (30 vértices - 10 triángulos)
		 // ==========================================
		 // 1. Barra horizontal superior
		  0.45f,  0.35f, 0.0f,   0.95f,  0.35f, 0.0f,   0.95f,  0.45f, 0.0f,
		  0.45f,  0.35f, 0.0f,   0.95f,  0.45f, 0.0f,   0.45f,  0.45f, 0.0f,

		  // 2. Barra vertical superior izquierda
		   0.45f,  0.05f, 0.0f,   0.55f,  0.05f, 0.0f,   0.55f,  0.35f, 0.0f,
		   0.45f,  0.05f, 0.0f,   0.55f,  0.35f, 0.0f,   0.45f,  0.35f, 0.0f,

		  // 3. Barra horizontal media
			0.45f, -0.05f, 0.0f,   0.95f, -0.05f, 0.0f,   0.95f,  0.05f, 0.0f,
			0.45f, -0.05f, 0.0f,   0.95f,  0.05f, 0.0f,   0.45f,  0.05f, 0.0f,

			// 4. Barra vertical inferior derecha
			 0.85f, -0.35f, 0.0f,   0.95f, -0.35f, 0.0f,   0.95f, -0.05f, 0.0f,
			 0.85f, -0.35f, 0.0f,   0.95f, -0.05f, 0.0f,   0.85f, -0.05f, 0.0f,

			 // 5. Barra horizontal inferior
			  0.45f, -0.45f, 0.0f,   0.95f, -0.45f, 0.0f,   0.95f, -0.35f, 0.0f,
			  0.45f, -0.45f, 0.0f,   0.95f, -0.35f, 0.0f,   0.45f, -0.35f, 0.0f
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posición en location = 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	GLuint theShader = glCreateShader(shaderType);
	const GLchar* theCode[1];
	theCode[0] = shaderCode;
	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);

	glShaderSource(theShader, 1, theCode, codeLength);
	glCompileShader(theShader);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("El error al compilar el shader %d es: %s \n", shaderType, eLog);
		return;
	}

	glAttachShader(theProgram, theShader);
}

void CompileShaders()
{
	shader = glCreateProgram();
	if (!shader)
	{
		printf("Error creando el shader\n");
		return;
	}

	AddShader(shader, vShader, GL_VERTEX_SHADER);
	AddShader(shader, fShader, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("El error al linkear es: %s \n", eLog);
		return;
	}

	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("El error al validar es: %s \n", eLog);
		return;
	}
}

int main()
{
	srand(time(NULL));
	// Inicialización de GLFW
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW\n");
		glfwTerminate();
		return 1;
	}

	// Asignando variables de GLFW y propiedades de ventana
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// CREAR VENTANA
	GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Cuadrado y Rombo con Fondo RGB", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW\n");
		glfwTerminate();
		return 1;
	}

	// Obtener tamaño de Buffer
	int BufferWidth, BufferHeight;
	glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight);

	// Asignar el contexto
	glfwMakeContextCurrent(mainWindow);

	// Permitir nuevas extensiones
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Falló inicialización de GLEW\n");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	// Asignar Viewport
	glViewport(0, 0, BufferWidth, BufferHeight);

	// Llamada a las funciones creadas antes del main
	CrearFiguras();
	CompileShaders();

	// Loop mientras no se cierra la ventana
	while (!glfwWindowShouldClose(mainWindow))
	{
		// Recibir eventos del usuario
		glfwPollEvents();

		// -------------------------------------------------------------
		// 1. Cambio de fondo cíclico (Rojo -> Verde -> Azul)
		// -------------------------------------------------------------
		double tiempo = glfwGetTime();
		int estado_actual = (((int)tiempo) / 2) % 3;
		static int estado_anterior = -1;

		if (estado_actual != estado_anterior) {
			float r = (float)rand() / RAND_MAX;
			float g = (float)rand() / RAND_MAX;
			float b = (float)rand() / RAND_MAX;

			glClearColor(r, g, b, 1.0f);

			estado_anterior = estado_actual;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		// -------------------------------------------------------------
		// 2. Dibujar figuras
		// -------------------------------------------------------------
		glUseProgram(shader);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 78);
		glBindVertexArray(0);

		glUseProgram(0);

		glfwSwapBuffers(mainWindow);
		// NO ESCRIBIR NINGUNA LÍNEA DESPUÉS DE glfwSwapBuffers(mainWindow); 
	}

	return 0;
}