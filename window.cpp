#include "window.h"
#include <iostream>
#include <vector>
#include <random>
#include "shader.h"
#include "error.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


glm::vec3 viewPos(0,2,16);
glm::vec3 cammera_front(-3.0f, 2.5f, -15.0f);
glm::vec3 lightPos();
static int sphere_no_triangle = 0;

// positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3( -5, 20, 8.0),
    glm::vec3(-5,  20, -8.0),
    glm::vec3(15.0f,2.5f, 8.0f),
    glm::vec3(15.0f, 2.5f, -8.0f)
};
glm::vec3 pointLightPositions_bk[]= {
    glm::vec3( -5, 20, 8.0),
    glm::vec3(-5,  20, -8.0),
    glm::vec3(15.0f,3.5f, 8.0f),
    glm::vec3(15.0f, 3.5f, -8.0f)
};
static int roateFlag = 0;
static int light_flag = 0; // 0: dirLight, 1:PointLight,2:spotLight
static int light_model_flag; // 0:phong,1:cook-torrance,2:Oren-Nyar, 3:Ward
static bool gouraudFlag = false;
static bool rolate_light = false;
static int draw_norm_flag = 0;


Window::Window(int width, int height, const string& title)
{
	// Initialise GLFW
	glfwSetErrorCallback(glfwErrorCB);
	if( !glfwInit() )
	{
		cerr << "Failed to initialize GLFW"<<endl;
		getchar();
	}
    std::cout << "Current path is " << std::__fs::filesystem::current_path()<< std::endl;


	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if( window == NULL ){
		cerr <<"Failed to open GLFW window." << endl;
		getchar();
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
    std::cout << "Running OpenGL Version " << glGetString (GL_VERSION)
              << " using " << glGetString (GL_RENDERER) << "\n";
    // Initialize GLEW
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		getchar();
		glfwTerminate();
	}

    // Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	//VAO
	glGenVertexArrays(1, &worldVAO);
	glBindVertexArray(worldVAO);
    
  
    


    //shader
    Shader shader;
    phongProgramID = shader.Load( "shader/PhongVtx.glsl",
                                 "shader/PhongFrag.glsl" );
    worldProgramID = phongProgramID;
    
    gouraudProgramID = shader.Load( "shader/gouraudVtx.glsl",
                                 "shader/gouraudFrag.glsl" );

    
    lightProgramID= shader.Load( "shader/lambvertex.glsl",
                                "shader/lambfreg.glsl" );
    
//    modelProgramID= shader.Load( "shader/model_loading_vs.glsl",
//                                "shader/model_loading_fs.glsl" );
    
    loadModel("res/nanosuit/nanosuit.obj");
    loadModelIronMan("res/IronMan/IronMan.obj");

            // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    // Clear the screen
    glEnable(GL_ALPHA_TEST);
            // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    angle = 0;
    speed = 0.002f;
    bgColor[0]=0.1f;
    bgColor[1]=0.1f;
    bgColor[2]=0.1f;
    bgColor[3]=1.0f;


	Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    View = glm::lookAt(
                            viewPos, // Camera is at (4,3,-3), in World Space
                            glm::vec3(0,0,0), // and looks at the origin
                            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                        );



    static const float platonic_vertices[10][3]=
    {

      //Tetrahedron and cube :
      {1,1,1},{-1,1,1},{-1,-1,1},{-1,-1,-1},{1,-1,1},{1,-1,-1},{1,1,-1},{-1,1,-1},
      //Octahedron:
      {0,0,0},{0,2,0},
    };


    static const int cube_triangles[12][3]=
    {
        //top
        {6,7,1},
        {6,1,0},
        //right
        {6,0,4},
        {6,4,5},
        //back
        {6,3,7},
        {6,5,3},
        //front
        {0,1,2},
        {0,2,4},
        //left
        {7,2,1},
        {7,3,2},
        //bottom
        {5,2,3},
        {5,4,2}

     };






    /********************************************************************
    *Cube start
    **/

    static int v=0;
    glm::vec3 a,b,c; // temp vertices of a triangle
    static  GLfloat cube_norms_buffer_data[12*3*6];
    static  GLfloat cube_vertex_buffer_data[12*3*6];
    for (int i = 0; i < 12;i++)
    {

        a =  glm::make_vec3(platonic_vertices[cube_triangles[i][0]]);
        b =  glm::make_vec3(platonic_vertices[cube_triangles[i][1]]);
        c =  glm::make_vec3(platonic_vertices[cube_triangles[i][2]]);



        glm::vec3 normal = glm::normalize(glm::cross(b - a,c - a));


        for(int j=0;j<3;j++)
        {
            cube_vertex_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][0];
            cube_norms_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][0];
            ++v;
            cube_vertex_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][1];
            cube_norms_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][1];
            ++v;
            cube_vertex_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][2];
            cube_norms_buffer_data[v]= platonic_vertices[cube_triangles[i][j]][2];


            ++v;
            cube_vertex_buffer_data[v] = normal[0];
            cube_norms_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][0]+normal[0];
            ++v;
            cube_vertex_buffer_data[v] = normal[1];
            cube_norms_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][1]+normal[1];
            ++v;
            cube_vertex_buffer_data[v] = normal[2];
            cube_norms_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][2]+normal[2];
            ++v;
        }

    }
//
//    for(int i=0;i<12*3*6;i++)
//    {
//        if(i%6==0)
//        std::cout<<endl;
//        std::cout<< cube_vertex_buffer_data[i] << ",";
//
//    }
//


    glGenBuffers(1, &cube_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_buffer_data), cube_vertex_buffer_data, GL_STATIC_DRAW);
    glGenBuffers(1, &cube_norms_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, cube_norms_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norms_buffer_data), cube_norms_buffer_data, GL_STATIC_DRAW);


    /********************************************************************
    *Cube end
    **/

    
    
    /********************************************************************
    *tetrahedron start
    **/
    static const int tetrahedron_triangles[4][3]=
    {
        //Tetrahedron
        {0,7,5},
        {7,2,5},
        {0,5,2},
        {0,2,7},

     };

    v=0;
    static  GLfloat tetrahedron_norms_buffer_data[4*3*6];
    static  GLfloat tetrahedron_vertex_buffer_data[4*3*6];
    for (int i = 0; i < 4;i++)
    {
        a =  glm::make_vec3(platonic_vertices[tetrahedron_triangles[i][0]]);
        b =  glm::make_vec3(platonic_vertices[tetrahedron_triangles[i][1]]);
        c =  glm::make_vec3(platonic_vertices[tetrahedron_triangles[i][2]]);

        glm::vec3 normal = glm::normalize(glm::cross(c - a,b - a));
        for(int j=0;j<3;j++)
        {
            tetrahedron_vertex_buffer_data[v] = platonic_vertices[tetrahedron_triangles[i][j]][0];
            tetrahedron_norms_buffer_data[v]= platonic_vertices[tetrahedron_triangles[i][j]][0];
            ++v;
            tetrahedron_vertex_buffer_data[v] = platonic_vertices[tetrahedron_triangles[i][j]][1];
            tetrahedron_norms_buffer_data[v] = platonic_vertices[tetrahedron_triangles[i][j]][1];
            ++v;
            tetrahedron_vertex_buffer_data[v] = platonic_vertices[tetrahedron_triangles[i][j]][2];
            tetrahedron_norms_buffer_data[v] = platonic_vertices[tetrahedron_triangles[i][j]][2];

            
            ++v;
            tetrahedron_vertex_buffer_data[v] = normal[0];
            tetrahedron_norms_buffer_data[v]= platonic_vertices[tetrahedron_triangles[i][j]][0]+normal[0];
            ++v;
            tetrahedron_vertex_buffer_data[v] = normal[1];
            tetrahedron_norms_buffer_data[v] = platonic_vertices[tetrahedron_triangles[i][j]][1]+normal[1];
            ++v;
            tetrahedron_vertex_buffer_data[v] = normal[2];
            tetrahedron_norms_buffer_data[v] = platonic_vertices[tetrahedron_triangles[i][j]][2]+normal[2];
            ++v;
        }

    }


    glGenBuffers(1, &tetrahedron_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tetrahedron_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedron_vertex_buffer_data), tetrahedron_vertex_buffer_data, GL_STATIC_DRAW);
    glGenBuffers(1, &tetrahedron_norms_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, tetrahedron_norms_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedron_norms_buffer_data), tetrahedron_norms_buffer_data, GL_STATIC_DRAW);


    /********************************************************************
    *tetrahedron end
    **/


    /********************************************************************
     *Octahedron start
     */
    static const int octahedron_triangles[8][3]=
    {
        //top
        {6,9,0},
        {7,9,6},
        {7,1,9},
        {1,0,9},
        //bottom
        {8,0,1},
        {8,6,0},
        {8,7,6},
        {8,1,7}

     };

     v=0;
     static  GLfloat octahedron_norms_buffer_data[8*3*6];
     static  GLfloat octahedron_vertex_buffer_data[8*3*6];
     for (int i = 0; i < 8;i++)
     {
         
         a =  glm::make_vec3(platonic_vertices[octahedron_triangles[i][0]]);
         b =  glm::make_vec3(platonic_vertices[octahedron_triangles[i][1]]);
         c =  glm::make_vec3(platonic_vertices[octahedron_triangles[i][2]]);

         glm::vec3 normal = glm::normalize(glm::cross(b - a,c - a));
         for(int j=0;j<3;j++)
         {
             octahedron_vertex_buffer_data[v] = platonic_vertices[octahedron_triangles[i][j]][0];
             octahedron_norms_buffer_data[v]= platonic_vertices[octahedron_triangles[i][j]][0];
             ++v;
             octahedron_vertex_buffer_data[v] = platonic_vertices[octahedron_triangles[i][j]][1];
             octahedron_norms_buffer_data[v] = platonic_vertices[octahedron_triangles[i][j]][1];
             ++v;
             octahedron_vertex_buffer_data[v] = platonic_vertices[octahedron_triangles[i][j]][2];
             octahedron_norms_buffer_data[v] = platonic_vertices[octahedron_triangles[i][j]][2];
             ++v;
             
         
             octahedron_vertex_buffer_data[v] = normal[0];
             octahedron_norms_buffer_data[v]=platonic_vertices[octahedron_triangles[i][j]][0]+normal[0];
             ++v;
             octahedron_vertex_buffer_data[v] = normal[1];
             octahedron_norms_buffer_data[v] = platonic_vertices[octahedron_triangles[i][j]][1]+normal[1];
             ++v;
             octahedron_vertex_buffer_data[v] = normal[2];
             octahedron_norms_buffer_data[v] = platonic_vertices[octahedron_triangles[i][j]][2]+normal[2];
             ++v;
         }

     }

     glGenBuffers(1, &octahedron_vertexbuffer);
     glBindBuffer(GL_ARRAY_BUFFER, octahedron_vertexbuffer);
     glBufferData(GL_ARRAY_BUFFER, sizeof(octahedron_vertex_buffer_data), octahedron_vertex_buffer_data, GL_STATIC_DRAW);
    
    glGenBuffers(1, &octahedron_norm_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, octahedron_norm_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(octahedron_norms_buffer_data), octahedron_norms_buffer_data, GL_STATIC_DRAW);

    /********************************************************************
     *Octahedron end
     */

    
    /********************************************************************
    *Dodecahedron start
    **/



    static const GLfloat dodecahedron_vertices[20][3]=
    {
        // (±1, ±1, ±1)
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f},

        // (0, ±1.0f/φ, ±φ)
         {0.0f, -0.618f, -1.618f},
         {0.0f, -0.618f, 1.618f},
         {0.0f, 0.618f, -1.618f},
         { 0.0f, 0.618f, 1.618f},

        // (±1.0.0ff/φ, ±φ, 0.0f)
        {-0.618f, -1.618f, 0.0f},
        {-0.618f, 1.618f, 0.0f},
        {0.618f, -1.618f, 0.0f},
        { 0.618f, 1.618f, 0.0f},

        // (±φ, 0.0f, ±1.0.0ff/φ)
        {-1.618f, 0.0f, -0.618f},
        { 1.618f, 0.0f, -0.618f},
        {-1.618f, 0.0f, 0.618f},
        {1.618f, 0.0f, 0.618f}
    };



    static const int dodecahedron_triangles[36][3] =
        {
            {3, 11, 7},{3, 7, 15},{3, 15, 13},
            {7, 19, 17},{7, 17, 6},{7, 6, 15},
            {17, 4, 8},{17, 8, 10},{17, 10, 6},
            {8, 0, 16},{8, 16, 2},{8, 2, 10},
            {0, 12, 1},{0, 1, 18},{0, 18, 16},
            {6, 10, 2},{6, 2, 13},{6, 13, 15},
            {2, 16, 18},{2, 18, 3},{2, 3, 13},
            {18, 1, 9},{18, 9, 11},{18, 11, 3},
            {4, 14, 12},{4, 12, 0},{4, 0, 8},
            {11, 9, 5},{11, 5, 19},{11, 19, 7},
            {19, 5, 14},{19, 14, 4},{19, 4, 17},
            {1, 12, 14},{1, 14, 5},{1, 5, 9}
        };



     v=0;
    static  GLfloat dodecahedron_norms_buffer_data[36*3*6];
    static  GLfloat dodecahedron_vertex_buffer_data[36*3*6];
        for (int i = 0; i < 36;i++)
        {
            a =  glm::make_vec3(dodecahedron_vertices[dodecahedron_triangles[i][2]]);
            b =  glm::make_vec3(dodecahedron_vertices[dodecahedron_triangles[i][1]]);
            c =  glm::make_vec3(dodecahedron_vertices[dodecahedron_triangles[i][0]]);

            glm::vec3 normal = glm::normalize(glm::cross(c - a,b - a));
            for(int j=2;j>=0;j--)
            {
                dodecahedron_vertex_buffer_data[v] = dodecahedron_vertices[dodecahedron_triangles[i][j]][0]; //x
                dodecahedron_norms_buffer_data[v] = dodecahedron_vertices[dodecahedron_triangles[i][j]][0];
                ++v;
                dodecahedron_vertex_buffer_data[v] = dodecahedron_vertices[dodecahedron_triangles[i][j]][1]; //y
                dodecahedron_norms_buffer_data[v] = dodecahedron_vertices[dodecahedron_triangles[i][j]][1];
                ++v;
                dodecahedron_vertex_buffer_data[v] = dodecahedron_vertices[dodecahedron_triangles[i][j]][2]; //z
                dodecahedron_norms_buffer_data[v] = dodecahedron_vertices[dodecahedron_triangles[i][j]][2];

                ++v;
                dodecahedron_vertex_buffer_data[v] = normal[0];
                dodecahedron_norms_buffer_data[v] = dodecahedron_vertices[dodecahedron_triangles[i][j]][0] + normal[0];
                ++v;
                dodecahedron_vertex_buffer_data[v] = normal[1];
                dodecahedron_norms_buffer_data[v] =dodecahedron_vertices[dodecahedron_triangles[i][j]][1] + normal[1];
                ++v;
                dodecahedron_vertex_buffer_data[v] = normal[2];
                dodecahedron_norms_buffer_data[v] = dodecahedron_vertices[dodecahedron_triangles[i][j]][2] + normal[2];
                ++v;
                
                
            }

        }


    glGenBuffers(1, &dodecahedron_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, dodecahedron_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dodecahedron_vertex_buffer_data), dodecahedron_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &dodecahedron_norms_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, dodecahedron_norms_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dodecahedron_norms_buffer_data), dodecahedron_norms_buffer_data, GL_STATIC_DRAW);

    /********************************************************************
    *Dodecahedron end
    ****************************************************************/

    
    /********************************************************************
    *Icosahedron start
    **/


    const float X=.525731112119133606f;
    const float Z=.850650808352039932f;
    const float N=0.f;

    static const float icosahedron_vertices[12][3]=
    {
      {-X,N,Z}, {X,N,Z}, {-X,N,-Z}, {X,N,-Z},
      {N,Z,X}, {N,Z,-X}, {N,-Z,X}, {N,-Z,-X},
      {Z,X,N}, {-Z,X, N}, {Z,-X,N}, {-Z,-X, N}
    };

    static const int icosahedron_triangles[20][3]=
        {
          {0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
          {8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
          {7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
          {6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
        };

      
     v=0;
    static  GLfloat icosahedron_normals_buffer_data[20*3*6];
    static  GLfloat icosahedron_vertex_buffer_data[20*3*6];
        for (int i = 0; i < 20;i++)
        {
            a =  glm::make_vec3(icosahedron_vertices[icosahedron_triangles[i][0]]);
            b =  glm::make_vec3(icosahedron_vertices[icosahedron_triangles[i][1]]);
            c =  glm::make_vec3(icosahedron_vertices[icosahedron_triangles[i][2]]);

            glm::vec3 normal = glm::normalize(glm::cross(c - a,b - a));
            for(int j=0;j<3;j++)
            {
                icosahedron_vertex_buffer_data[v] = icosahedron_vertices[icosahedron_triangles[i][j]][0];
                icosahedron_normals_buffer_data[v] = icosahedron_vertices[icosahedron_triangles[i][j]][0];
                ++v;
                icosahedron_vertex_buffer_data[v] = icosahedron_vertices[icosahedron_triangles[i][j]][1];
                icosahedron_normals_buffer_data[v] = icosahedron_vertices[icosahedron_triangles[i][j]][1];
                ++v;
                icosahedron_vertex_buffer_data[v] = icosahedron_vertices[icosahedron_triangles[i][j]][2];
                icosahedron_normals_buffer_data[v]= icosahedron_vertices[icosahedron_triangles[i][j]][2];
                
                ++v;
                icosahedron_vertex_buffer_data[v] = normal[0];
                icosahedron_normals_buffer_data[v] = icosahedron_vertices[icosahedron_triangles[i][j]][0] + normal[0];
                ++v;
                icosahedron_vertex_buffer_data[v] = normal[1];
                icosahedron_normals_buffer_data[v] = icosahedron_vertices[icosahedron_triangles[i][j]][1] + normal[1];
                ++v;
                icosahedron_vertex_buffer_data[v] = normal[2];
                icosahedron_normals_buffer_data[v] = icosahedron_vertices[icosahedron_triangles[i][j]][2] + normal[2];
                ++v;
                
            }

        }


    glGenBuffers(1, &icosahedron_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, icosahedron_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(icosahedron_vertex_buffer_data), icosahedron_vertex_buffer_data, GL_STATIC_DRAW);
    
    glGenBuffers(1, &icosahedron_norms_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, icosahedron_norms_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(icosahedron_normals_buffer_data), icosahedron_normals_buffer_data, GL_STATIC_DRAW);

    /********************************************************************
    *Icosahedron end
    ****************************************************************/

   
    
    /**************************************************************
    *
    * House Start
    *
    *************************************************************
    */

    static const float vertices[51][3]=
    {
      // for roof
      {-2.0f,1.0f,0.0f}, {-2.0f,0.5f,-1.5f}, {-2.0f,0.5f,1.5f},
      {2.0f, 1.0f,0.0f},{2.0f,0.5f,-1.5f}, {2.0f,0.5f,1.5f},
      //for walls
      {-2.0,0.5,-1.4},{-2.0f,0.5f,1.4f},{2.0f,0.5f,1.4f},{2.0f,0.5f,-1.4f},  //top
      {-2.0,-1.0,-1.4},{-2.0f,-1.0,1.4f},{2.0f,-1.0,1.4f},{2.0f,-1.0,-1.4f},  //buttom
      //left window:
      {-1.5f,-0.5f,1.4f},{-1.1f,-0.5f,1.4f},{-1.1f,0.0f,1.4f},{-1.5f,0.0f,1.4f},
      //right window:
      {1.5f,-0.5f,1.4f},{1.5f,0.0f,1.4f},{1.1f,0.0f,1.4f},{1.1f,-0.5f,1.4f},

       //door:
      {-0.2f,-0.15f,1.4f},{0.2f,-0.15f,1.4f},{-0.2f,-1.0f,1.4f},{0.2f,-1.0f,1.4f},
      //front wall:
      {2.0f,0.0f,1.4f},{-2.0f,0.0f,1.4f},{-1.5f,-1.0f,1.4},{-0.2f,-0.5f,1.4},
      {0.2f,-0.5f,1.4f},{1.5f,-1.0f,1.4f},{2.0f,-0.5f,1.4f},{1.1f,-0.15f,1.4f},
      {-1.1f,-0.15f,1.4f},
      //chimey
      {-1.2,1.5,0.5f},{-1.2f, 1.5f,0.2f},{-1.5f,1.5f,0.2f},{-1.5f,1.5f,0.5f},
      {-1.2f,0.7f,0.5f},{-1.2f,0.7f,0.2f},{-1.5f,0.7f,0.2f},{-1.5f,0.7f,0.5f},
      //base:
      {2.3f,-1.0f,1.8f},{2.3f,-1.0f,-1.8f},{-2.3,-1.0f,-1.8f},{-2.3f,-1.0f,1.8f},
      {-2.3f,-1.3f,1.8f},{-2.3f,-1.3f,-1.8f},{2.3f,-1.3f,1.8f},{2.3f,-1.3f,-1.8f}

    };


    static const int triangles[house_triangles_no][3]=
        {
            //***************roof start******************
            {3,0,1}, //back side of the roof
            {3,1,4},
            {3,2,0}, // front of the roof
            {3,5,2},
            {0,2,1}, // left side roof
            {3,4,5},// rigt side roof
            //***************roof end******************
            //***********wall start***********
            //left side wall:
            {7,10,6},
            {7,11,10},
            //right side wall:
            {8,9,13},
            {8,13,12},
            //back side wall:
            {9,6,10},
            {9,10,13},
            //***********wall end*************

            //***********windows and doorstart**********
            //left window:
            {16,14,17},
            {16,15,14},
            //right window:
            {19,21,20},
            {19,18,21},
            //door:
            {23,24,22},
            {23,25,24},
            //***********windows end door*************
            //***********fron wall start**********
            //top
            {8,27,7},
            {8,26,27},
            //left
            {17,11,27},
            {17,28,11},
            //left down
            {29,28,14},
            {29,24,28},
            //left between window and door
            {22,15,34},
            {22,29,15},
            //right
            {26,31,19},
            {26,12,31},
            //right between window and door
            {33,30,23},
            {33,21,30},
            //right down
            {32,25,30},
            {32,31,25},

            //between window and up door
            {20,34,16},
            {20,33,34},

            //***********front wall end************

            //chimney
            {35,37,36},
            {35,38,37},
            {35,42,38},
            {35,39,42},
            {37,42,38},
            {37,41,42},
            {36,37,41},
            {36,41,40},
            {36,35,39},
            {36,39,40},
            
            //floor:
            {13,10,11},
            {13,11,12},
            //base:
            {43,45,44},
            {43,46,45},
            
            {44,45,48},
            {44,48,50},
            
            //base right
            {45,46,48},
            {47,48,46},
            
            {43,47,46},
            {43,49,47},
            
            {49,44,50},
            {49,43,44},
            
            {49,50,48},
            {49,48,47}


        };
            //VBO
    v=0;
    static GLfloat house_norms_vertex_buffer[house_triangles_no*3*6];
    
    static  GLfloat g_vertex_buffer_data[house_triangles_no*3*6];
        for (int i = 0; i < house_triangles_no;i++)
        {
            a =  glm::make_vec3(vertices[triangles[i][0]]);
            b =  glm::make_vec3(vertices[triangles[i][1]]);
            c =  glm::make_vec3(vertices[triangles[i][2]]);

            glm::vec3 normal = glm::normalize(glm::cross(c - a,b - a));
            
            for(int j=0;j<3;j++)
            {
                g_vertex_buffer_data[v] = vertices[triangles[i][j]][0];
                house_norms_vertex_buffer[v] = vertices[triangles[i][j]][0];
                ++v;
                g_vertex_buffer_data[v] = vertices[triangles[i][j]][1];
                house_norms_vertex_buffer[v] = vertices[triangles[i][j]][1];
                ++v;
                g_vertex_buffer_data[v] = vertices[triangles[i][j]][2];
                house_norms_vertex_buffer[v] = vertices[triangles[i][j]][2];
                
                ++v;
                g_vertex_buffer_data[v] = normal[0];
                house_norms_vertex_buffer[v] = normal[0] + vertices[triangles[i][j]][0];
                ++v;
                g_vertex_buffer_data[v] = normal[1];
                house_norms_vertex_buffer[v] = normal[1] + vertices[triangles[i][j]][1];
                ++v;
                g_vertex_buffer_data[v] = normal[2];
                house_norms_vertex_buffer[v] = normal[2]+ vertices[triangles[i][j]][2];
                ++v;
            }

        }



    glGenBuffers(1, &house_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, house_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    
    glGenBuffers(1, &house_norms_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, house_norms_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(house_norms_vertex_buffer), house_norms_vertex_buffer, GL_STATIC_DRAW);
    
    
    /**************************************************************
    *
    * House end
    *
    **************************************************************/
    
    
    /**************************************************************
    *
    * sphere start
    *
    *************************************************************/
    const int na=36;        // vertex grid size
    const int nb=18;
    const int nn=nb*na;    // whole grid size
    GLfloat sphere_pos[nn][6]; // vertex
    GLuint  sphere_ix [na*(nb-1)*6];    // indices
    
    // generate the sphere data
    GLfloat x1,y1,z1,a1,b1,da,db,r=1.5;
    int ia,ib,ix,iy;
    da=2.0*M_PI/GLfloat(na);
    db=    M_PI/GLfloat(nb-1);
    static int sp_v=0;
    for (ix=0,b1=-0.5*M_PI,ib=0;ib<nb;ib++,b1+=db)
     for (a1=0.0,ia=0;ia<na;ia++,a1+=da,ix+=3)
        {
        // unit sphere
        x1=cos(b1)*cos(a1);
        y1=cos(b1)*sin(a1);
        z1=sin(b1);
        sphere_pos[sp_v][0]=x1*r;
        sphere_pos[sp_v][1]=y1*r;
        sphere_pos[sp_v][2]=z1*r;
        sphere_pos[sp_v][3]=x1;
        sphere_pos[sp_v][4]=y1;
        sphere_pos[sp_v][5]=z1;
            ++sp_v;
        }
    // [Generate GL_TRIANGLE indices]
    for (ix=0,iy=0,ib=1;ib<nb;ib++)
        {
        for (ia=1;ia<na;ia++,iy++)
            {
            // first half of QUAD
            sphere_ix[ix]=iy;      ix++;
            sphere_ix[ix]=iy+1;    ix++;
            sphere_ix[ix]=iy+na;   ix++;
            // second half of QUAD
            sphere_ix[ix]=iy+na;   ix++;
            sphere_ix[ix]=iy+1;    ix++;
            sphere_ix[ix]=iy+na+1; ix++;
            }
        // first half of QUAD
        sphere_ix[ix]=iy;       ix++;
        sphere_ix[ix]=iy+1-na;  ix++;
        sphere_ix[ix]=iy+na;    ix++;
        // second half of QUAD
        sphere_ix[ix]=iy+na;    ix++;
        sphere_ix[ix]=iy-na+1;  ix++;
        sphere_ix[ix]=iy+1;     ix++;
        iy++;
        }
    
    static  GLfloat sp_vertex_buffer_data[na*(nb-1)*6*6];
    v=0;
    for(int i=0;i<na*(nb-1)*6;i++)
    {
        for(int j=0;j<6;j++)
        {
            sp_vertex_buffer_data[v] = sphere_pos[sphere_ix[i]][j];
            ++v;
        }
    }
    
    v=0;
    static  GLfloat sp_vertex_norms_data[na*(nb-1)*6*6];
    for(int i=0;i<na*(nb-1)*6;i++)
    {
  
        sp_vertex_norms_data[v] = sphere_pos[sphere_ix[i]][0];
        ++v;
        sp_vertex_norms_data[v] = sphere_pos[sphere_ix[i]][1];
        ++v;
        sp_vertex_norms_data[v] = sphere_pos[sphere_ix[i]][2];
        ++v;
        
        
        sp_vertex_norms_data[v] = sphere_pos[sphere_ix[i]][0] + sphere_pos[sphere_ix[i]][3];
        ++v;
        sp_vertex_norms_data[v] = sphere_pos[sphere_ix[i]][1] + sphere_pos[sphere_ix[i]][4];
        ++v;
        sp_vertex_norms_data[v] = sphere_pos[sphere_ix[i]][2] + sphere_pos[sphere_ix[i]][5];
        ++v;
    }
    
    
    glGenBuffers(1, &sphere_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sp_vertex_buffer_data), sp_vertex_buffer_data, GL_STATIC_DRAW);
    
    glGenBuffers(1, &sphere_norms_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_norms_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sp_vertex_norms_data), sp_vertex_norms_data, GL_STATIC_DRAW);
    sphere_no_triangle =na*(nb-1)*6*3;




    /**************************************************************
    *
    * sphere end
    *
    **************************************************************/
    
    
    
    
    
    
    
    


    
    /********************************************************************
    *light  start
    ****************************************************************/

    v=0;
    
    static  GLfloat light_vertex_buffer_data[12*3*3];
    for (int i = 0; i < 12;i++)
    {

        for(int j=0;j<3;j++)
        {
            light_vertex_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][0];
            ++v;
            light_vertex_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][1];
            ++v;
            light_vertex_buffer_data[v] = platonic_vertices[cube_triangles[i][j]][2];
            ++v;
        }

    }
    glGenBuffers(1, &light_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, light_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertex_buffer_data), light_vertex_buffer_data, GL_STATIC_DRAW);


    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, light_vertexbuffer);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    /********************************************************************
    *light   end
    ****************************************************************/

    
    
    
    


}



glm::vec3 Window::computeSphereVertices(float theta, float phi)
{
    float x = r * sin(theta * degToRad) * sin(phi * degToRad);
    float y = r * cos(theta * degToRad);
    float z = r * sin(theta * degToRad) * cos(phi * degToRad);
    
    return glm::vec3(x,y,z);
}


void Window::setMat4(const GLuint ID, std::string name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}



void Window::setVec3(const GLuint ID,  const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Window::setFloat(const GLuint ID,const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Window::setInt(const GLuint ID,const std::string &name, int value) const
{
     glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}


void Window::TransformLight()
{

	// Model matrix : an identity matrix (model will be at the origin)
	Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices

//	glm::vec3 lightPos(1.5f,3.0f,1.0f);

    //setVec3(lightProgramID,"lightPos",lightPos);
    //setVec3(worldProgramID,"viewPos", glm::vec3(-3.5,2,10));
	Model = glm::scale(Model,glm::vec3(0.2f,0.2f,0.2f));

//	Model = glm::rotate(Model, angle, glm::vec3(0.1 ,0.0, 0.0));
    Model =  glm::translate(Model,lightPos);
    setMat4(lightProgramID,"model", Model);
    setMat4(lightProgramID,"view", View);
    setMat4(lightProgramID,"projection", Projection);
//    lightPos = glm::vec3(Model * vec4(lightPos, 1.0));

}



void Window::RenderLight()
{
    
    for(int i=0;i<4;i++)
        pointLightPositions[i] = pointLightPositions_bk[i];

    for(int i=0;i<4;i++)
    {

        if(rolate_light)
        {
            float lightX = 20.0f * sin(glfwGetTime());
            float lightY = 0.0f;
            float lightZ = 35 * cos(glfwGetTime());
            lightPos = pointLightPositions[i]+ glm::vec3(lightX, lightY, lightZ);
            pointLightPositions[i] = lightPos;
            
        }
        else
        {
           lightPos = pointLightPositions[i];
        }
        TransformLight();
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, light_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,  // stride
			(void*)0            // array buffer offset
		);

        glDrawArrays(GL_TRIANGLES, 0, 12*3);
        glDisableVertexAttribArray(0);
        
    }

}


void Window::TransformCube()
{


    setVec3(worldProgramID,"viewPos", viewPos);
    //    lightPos = glm::vec3(Model * glm::vec4(lightPos, 1.0));


	// Model matrix : an identity matrix (model will be at the origin)
    Model = glm::mat4(1.0f);
    Model = glm::scale(Model,glm::vec3(0.5f,0.5f,0.5f));
    Model =  glm::translate(Model,glm::vec3(6.5f,5.0f,-1.0f));
    Model = glm::rotate(Model, 15.0f, glm::vec3(1.0 ,0.0, 0.0));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));
    //MVP   = Projection * View * Model; // Remember, matrix multiplication is the other way around
    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);
    // pearl
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.25f, 0.20725f, 0.20725f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(1.0f,0.829f, 0.829f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.296648f,0.296648f, 0.296648f));
    setFloat(worldProgramID,"material.shininess",11.264);

    //setVec3(worldProgramID,"light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
}


void Window::RenderCube()
{
        TransformCube();
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, cube_vertexbuffer);
		glVertexAttribPointer(
			0,                  //
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			6 * sizeof(float),                  // stride
			(void*)0            // array buffer offset
		);


        glVertexAttribPointer(
			1,                  // for the noramal
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			6 * sizeof(float),  // stride
			(void*)(3 * sizeof(float))            // array buffer offset
		);



        glDrawArrays(GL_TRIANGLES, 0, 12*3);
        glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	

}



void Window::TransformTetrahedron()
{
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    Model = glm::scale(Model,glm::vec3(0.5f,0.5f,0.5f));
    Model =  glm::translate(Model,glm::vec3(12.0f,5.0f,-1.0f));
    Model = glm::rotate(Model, 60.0f, glm::vec3(1.5,2.5, 0.0));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));
    
    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);

    //gold
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.24725f, 0.1995f, 0.0745f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.75164f,0.60648f, 0.22648f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.628281f,0.555802f, 0.366065f));
    setFloat(worldProgramID,"material.shininess",51.2);



}


void Window::RenderTetrahedron()
{
        TransformTetrahedron();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, tetrahedron_vertexbuffer);
        glVertexAttribPointer(
            0,                  //
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),                  // stride
            (void*)0            // array buffer offset
        );


        glVertexAttribPointer(
            1,                  // for the noramal
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),  // stride
            (void*)(3 * sizeof(float))            // array buffer offset
        );

        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLES, 0, 4*3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

}


void Window::TransformOctahedron()
{
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    Model = glm::scale(Model,glm::vec3(0.6f,0.6f,0.6f));
    Model =  glm::translate(Model,glm::vec3(2.0f,3.0f,-1.0f));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));
    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);
    
    //brass
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.329412, 0.223529, 0.027451));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.780392,0.568627, 0.113725));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.992157,0.941176, 0.807843));
    setFloat(worldProgramID,"material.shininess",27.89743616);
}


void Window::RenderOctahedron()
{
        TransformOctahedron();

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, octahedron_vertexbuffer);
        glVertexAttribPointer(
            0,                  //
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),                  // stride
            (void*)0            // array buffer offset
        );


        glVertexAttribPointer(
            1,                  // for the noramal
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),  // stride
            (void*)(3 * sizeof(float))            // array buffer offset
        );
        glDrawArrays(GL_TRIANGLES, 0, 8*3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

}


void Window::TransformDodecahedron()
{

    // Model matrix : an identity matrix (model will be at the origin)
    Model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    Model = glm::scale(Model,glm::vec3(0.4f,0.4f,0.4f));
    Model =  glm::translate(Model,glm::vec3(-2.0f,6.0f,-1.0f));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));
    
    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);
    
    //gold
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.24725f, 0.1995f, 0.0745f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.75164f,0.60648f, 0.22648f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.628281f,0.555802f, 0.366065f));
    setFloat(worldProgramID,"material.shininess",51.2);
}


void Window::RenderDodecahedron()
{
        TransformDodecahedron();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, dodecahedron_vertexbuffer);
        glVertexAttribPointer(
            0,                  //
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),                  // stride
            (void*)0            // array buffer offset
        );
        glVertexAttribPointer(
            1,                  // for the noramal
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),  // stride
            (void*)(3 * sizeof(float))            // array buffer offset
        );
        glDrawArrays(GL_TRIANGLES, 0, 36*3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

}


void Window::TransformIcosahedron()
{


    // Model matrix : an identity matrix (model will be at the origin)
    Model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    Model = glm::scale(Model,glm::vec3(0.8f,0.8f,0.8f));
    Model =  glm::translate(Model,glm::vec3(-5.0f,4.0f,-1.0f));
    Model = glm::rotate(Model, 45.0f, glm::vec3(1.0,0.0, 0.0));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));
    
    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);
    
    //Emerald
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.0215f,0.1745f,0.0215f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.07568f,0.61424f,0.07568f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.633f,0.727811f,0.633f));
    setFloat(worldProgramID,"material.shininess",76.8f);
//    //gold
//    setVec3(worldProgramID,"material.ambient", glm::vec3(0.24725f, 0.1995f, 0.0745f));
//    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.75164f,0.60648f, 0.22648f));
//    setVec3(worldProgramID,"material.specular", glm::vec3(0.628281f,0.555802f, 0.366065f));
//    setFloat(worldProgramID,"material.shininess",51.2);
   
}


void Window::RenderIcosahedron()
{
        TransformIcosahedron();

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, icosahedron_vertexbuffer);
        glVertexAttribPointer(
            0,                  //
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),                  // stride
            (void*)0            // array buffer offset
        );
        glVertexAttribPointer(
            1,                  // for the noramal
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),  // stride
            (void*)(3 * sizeof(float))            // array buffer offset
        );
        glDrawArrays(GL_TRIANGLES, 0, 20*3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

}


void Window::TransformHouse()
{

    Model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
//    Model = glm::scale(Model,glm::vec3(0.6f,0.6f,0.6f));
    Model =  glm::translate(Model,glm::vec3(1.5f,-1.7f,2.0f));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));
    
    
    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);
    


}

void Window::RenderHouse()
{

        TransformHouse();

    //************roof start*********************
//    //Turquoise
//    setVec3(worldProgramID,"material.ambient", glm::vec3(0.1f, 0.18725f, 0.1745f));
//    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.396f,0.74151f, 0.69102f));
//    setVec3(worldProgramID,"material.specular", glm::vec3(0.297254f,0.30829f, 0.306678f));
//    setFloat(worldProgramID,"material.shininess",12.8);
    
    //Rubber (Black)
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.02f, 0.02f, 0.02f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.01f,0.01f, 0.01f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.4f,0.4f, 0.4f));
    setFloat(worldProgramID,"material.shininess",10);
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, house_vertexbuffer);
    glVertexAttribPointer(
        0,                  //
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        6 * sizeof(float),                  // stride
        (void*)0            // array buffer offset
    );
    glVertexAttribPointer(
        1,                  // for the noramal
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        6 * sizeof(float),  // stride
        (void*)(3 * sizeof(float))            // array buffer offset
    );
    glDrawArrays(GL_TRIANGLES, 0, 4*3);
    //************roof end*********************
    
    
    //************walls start*********************
    //silver
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.19225f, 0.19225f, 0.19225f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.50754f,0.50754f, 0.50754f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.508273f,0.508273f, 0.508273f));
    setFloat(worldProgramID,"material.shininess",51.2f);
    glVertexAttribPointer(
        0,                  //
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        6 * sizeof(float),                  // stride
        (void*)0            // array buffer offset
    );
    glVertexAttribPointer(
        1,                  // for the noramal
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        6 * sizeof(float),  // stride
        (void*)(3 * sizeof(float))            // array buffer offset
    );
    glDrawArrays(GL_TRIANGLES, 12, 8*3);
    //************wall end*********************
    
    
    //************windows start********************
    // pearl
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.25f, 0.20725f, 0.20725f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(1.0f,0.829f, 0.829f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.296648f,0.296648f, 0.296648f));
    setFloat(worldProgramID,"material.shininess",11.264);
    glVertexAttribPointer(
        0,                  //
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        6 * sizeof(float),                  // stride
        (void*)0            // array buffer offset
    );
    glVertexAttribPointer(
        1,                  // for the noramal
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        6 * sizeof(float),  // stride
        (void*)(3 * sizeof(float))            // array buffer offset
    );
    glDrawArrays(GL_TRIANGLES, 36, 4*3);
    //************windows end*********************
    
    //************door start********************
    //Plastic (Red)
      setVec3(worldProgramID,"material.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
      setVec3(worldProgramID,"material.diffuse", glm::vec3(0.5f,0.0f, 0.0f));
      setVec3(worldProgramID,"material.specular", glm::vec3(0.7f,0.6f, 0.6f));
      setFloat(worldProgramID,"material.shininess",32);
    glVertexAttribPointer(
        0,                  //
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        6 * sizeof(float),                  // stride
        (void*)0            // array buffer offset
    );
    glVertexAttribPointer(
        1,                  // for the noramal
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        6 * sizeof(float),  // stride
        (void*)(3 * sizeof(float))            // array buffer offset
    );
    glDrawArrays(GL_TRIANGLES, 48, 2*3);
    //************door end*********************
    
    
    
    
    
    //gold
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.24725f, 0.1995f, 0.0745f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.75164f,0.60648f, 0.22648f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.628281f,0.555802f, 0.366065f));
    setFloat(worldProgramID,"material.shininess",51.2);
    
        // 1rst attribute buffer : vertices

        glVertexAttribPointer(
            0,                  //
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),                  // stride
            (void*)(0)             // array buffer offset
        );
        glVertexAttribPointer(
            1,                  // for the noramal
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),  // stride
            (void*)(3 * sizeof(float))            // array buffer offset
        );
        glDrawArrays(GL_TRIANGLES, 54, (house_triangles_no-18)*3);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
}


void Window::drawNormls()
{
    setVec3(worldProgramID,"material.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
    setVec3(worldProgramID,"material.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    setFloat(worldProgramID,"material.shininess",100);
        //****************house normls*****************************
        TransformHouse();
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, house_norms_vertexbuffer);
        glVertexAttribPointer(
            0,                  //
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0       // array buffer offset
            );
        glDrawArrays(GL_LINES, 0, house_triangles_no*6);
        glDisableVertexAttribArray(0);
    
    //****************house normls******************************/
  
    TransformIcosahedron();
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, icosahedron_norms_buffer);
    glVertexAttribPointer(
        0,                  //
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0       // array buffer offset
        );
    glDrawArrays(GL_LINES, 0, 20*6);
    glDisableVertexAttribArray(0);
    
    
      //****************octahedron normls******************************/
    
    TransformOctahedron();
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, octahedron_norm_buffer);
      glVertexAttribPointer(
          0,                  //
          3,                  // size
          GL_FLOAT,           // type
          GL_FALSE,           // normalized?
          0,                  // stride
          (void*)0       // array buffer offset
          );
      glDrawArrays(GL_LINES, 0, 8*6);
      glDisableVertexAttribArray(0);
    
    //****************Tetrahedron normls******************************/
    
    TransformTetrahedron();
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, tetrahedron_norms_buffer);
      glVertexAttribPointer(
          0,                  //
          3,                  // size
          GL_FLOAT,           // type
          GL_FALSE,           // normalized?
          0,                  // stride
          (void*)0       // array buffer offset
          );
      glDrawArrays(GL_LINES, 0, 4*6);
      glDisableVertexAttribArray(0);
    
    
    //****************cube normls******************************/
    
    TransformCube();
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, cube_norms_buffer);
      glVertexAttribPointer(
          0,                  //
          3,                  // size
          GL_FLOAT,           // type
          GL_FALSE,           // normalized?
          0,                  // stride
          (void*)0       // array buffer offset
          );
      glDrawArrays(GL_LINES, 0, 12*6);
      glDisableVertexAttribArray(0);
    
    
    //****************dodecahedron normls******************************/
    
      TransformDodecahedron();
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, dodecahedron_norms_buffer);
      glVertexAttribPointer(
          0,                  //
          3,                  // size
          GL_FLOAT,           // type
          GL_FALSE,           // normalized?
          0,                  // stride
          (void*)0       // array buffer offset
          );
      glDrawArrays(GL_LINES, 0, 36*6);
      glDisableVertexAttribArray(0);
    
//****************sphere normls******************************/
    
      TransformSphere();
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, sphere_norms_buffer);
      glVertexAttribPointer(
          0,                  //
          3,                  // size
          GL_FLOAT,           // type
          GL_FALSE,           // normalized?
          0,                  // stride
          (void*)0       // array buffer offset
          );
      glDrawArrays(GL_LINES, 0, sphere_no_triangle*6);
      glDisableVertexAttribArray(0);
    
    
}








void Window::TransformSphere()
{

    Model = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    Model = glm::scale(Model,glm::vec3(1.5f,1.5f,1.5f));
    Model =  glm::translate(Model,glm::vec3(-4.0f,-2.0f,-1.0f));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));
    
    
    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);
    
    //gold
//    setVec3(worldProgramID,"material.ambient", glm::vec3(0.24725f, 0.1995f, 0.0745f));
//    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.75164f,0.60648f, 0.22648f));
//    setVec3(worldProgramID,"material.specular", glm::vec3(0.628281f,0.555802f, 0.366065f));
//    setFloat(worldProgramID,"material.shininess",51.2);

}

void Window::RenderSphere()
{
        TransformSphere();
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_vertexbuffer);

        glVertexAttribPointer(
            0,                  //
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),                  // stride
            (void*)0            // array buffer offset
        );
        glVertexAttribPointer(
            1,                  // for the noramal
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            6 * sizeof(float),  // stride
            (void*)(3 * sizeof(float))            // array buffer offset
        );
        glDrawArrays(GL_TRIANGLES, 0, sphere_no_triangle);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
}




void Window::TransformNanosui()
{

    Model = glm::mat4(1.0f);

    Model = glm::scale(Model,glm::vec3(0.4f,0.4f,0.4f));
    Model =  glm::translate(Model,glm::vec3(4.0f,-0.5f,0.0f));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));

    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);
    
    //silver
    setVec3(worldProgramID,"material.ambient", glm::vec3(0.19225f, 0.19225f, 0.19225f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.50754f,0.50754f, 0.50754f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.508273f,0.508273f, 0.508273f));
    setFloat(worldProgramID,"material.shininess",51.2f);
    



}

void Window::RenderNanosui()
{
    TransformNanosui();
    drawModelNanosui();
        
}

void Window::TransformIronMan()
{

    Model = glm::mat4(1.0f);

    Model = glm::scale(Model,glm::vec3(0.025f,0.025f,0.025f));
    Model =  glm::translate(Model,glm::vec3(-190.0f,-1.0f,0.0f));
    Model = glm::rotate(Model, angle, glm::vec3(0.0 ,-0.5, 0.0));

    setMat4(worldProgramID,"model", Model);
    setMat4(worldProgramID,"view", View);
    setMat4(worldProgramID,"projection", Projection);
    
        //gold

    setVec3(worldProgramID,"material.ambient", glm::vec3(0.24725f, 0.1995f, 0.0745f));
    setVec3(worldProgramID,"material.diffuse", glm::vec3(0.75164f,0.60648f, 0.22648f));
    setVec3(worldProgramID,"material.specular", glm::vec3(0.628281f,0.555802f, 0.366065f));
    setFloat(worldProgramID,"material.shininess",51.2);

}

void Window::RenderIranMan()
{
    TransformIronMan();
    drawModelIronMan();
}





void Window::Clear(float r, float g, float b,float a)
{
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT);
}


void Window::RenderWorld()
{
    angle = 0;
    static bool fillFlag = true;
    
	do{

        glClearColor(bgColor[0],bgColor[1],bgColor[2],bgColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Use our shader
        // Use our shader
        glBindVertexArray(worldVAO);
        glUseProgram(worldProgramID);

        configureLights();
        setInt(worldProgramID, "light_flag", light_flag);
        setInt(worldProgramID, "light_model_flag", light_model_flag);
        setInt(worldProgramID, "draw_norm_flag", draw_norm_flag);

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform

        if(gouraudFlag)
            worldProgramID = gouraudProgramID;
        else
            worldProgramID = phongProgramID;

        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
        {
            if(fillFlag)
                fillFlag = false;
            else
               fillFlag = true;

        }
        if (fillFlag)
        {
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        }else
        {
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }


    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
        {
            angle = 0;
            if(roateFlag==1)
                roateFlag = 0;
            else
               roateFlag = 1;

        }

        if (roateFlag==1)
        {
            angle += speed;
            if (angle > 360)
                angle = 0;
            
        }
        if((glfwGetKey(window, GLFW_KEY_KP_0 ) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_RELEASE) || (glfwGetKey(window, GLFW_KEY_0 ) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE))
            light_flag = 0;

        if((glfwGetKey(window, GLFW_KEY_KP_1 ) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_RELEASE) || (glfwGetKey(window, GLFW_KEY_1 ) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE))
            light_flag = 1;

        if((glfwGetKey(window, GLFW_KEY_KP_2 ) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_RELEASE) || (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE))
            light_flag = 2;
        if((glfwGetKey(window, GLFW_KEY_KP_3 ) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_RELEASE) || (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE))
            light_flag = 3;


        RenderCube();
        RenderTetrahedron();
        RenderOctahedron();
        RenderDodecahedron();
        RenderIcosahedron();
        RenderHouse();
        RenderSphere();
        
        if(draw_norm_flag==1)
                drawNormls();

//        glUseProgram(modelProgramID);
        RenderNanosui();
        RenderIranMan();
        
        if(light_flag==2 || light_flag==3)
        {
            glBindVertexArray(lightVAO);
            glUseProgram(lightProgramID);
            RenderLight();
        }
        
        
        
        if((glfwGetKey(window, GLFW_KEY_KP_4 ) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_RELEASE) || (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE))
        {
            if(rolate_light)
                rolate_light = false;
            else
               rolate_light = true;
 
        }
        
        if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
        {
            if(gouraudFlag)
                gouraudFlag = false;
            else
               gouraudFlag = true;

        }
        if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE)
        {
            if(draw_norm_flag==0)
                draw_norm_flag = 1;
            else
               draw_norm_flag = 0;
            if(fillFlag)
                 fillFlag = false;
             else
                fillFlag = true;

        }
        if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
        {
            light_model_flag = 0;
            light_flag = 3;
        }
        if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
        {
            light_model_flag = 1;
            light_flag = 3;
        }
        if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE)
        {
            light_model_flag = 2;
            light_flag = 3;
        }
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
        {
            light_model_flag = 3;
            light_flag = 3;
        }
        if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
        {
            light_model_flag = 4;
            light_flag = 3;
        }
        if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
        {
            light_model_flag = 5;
            light_flag = 3;
        }
        

		 //Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );
}


void Window::configureLights()
{
    setVec3(worldProgramID,"dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    setVec3(worldProgramID,"dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    setVec3(worldProgramID,"dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
    setVec3(worldProgramID,"dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    
    
    
    
    // point light 1
    setVec3(worldProgramID,"pointLights[0].position", pointLightPositions[0]);
    setVec3(worldProgramID,"pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    setVec3(worldProgramID,"pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    setVec3(worldProgramID,"pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
    setFloat(worldProgramID,"pointLights[0].constant", 1.0f);
    setFloat(worldProgramID,"pointLights[0].linear", 0.027);
    setFloat(worldProgramID,"pointLights[0].quadratic", 0.0028);

    // point light 2
    setVec3(worldProgramID,"pointLights[1].position", pointLightPositions[1]);
    setVec3(worldProgramID,"pointLights[1].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    setVec3(worldProgramID,"pointLights[1].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    setVec3(worldProgramID,"pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
    setFloat(worldProgramID,"pointLights[1].constant", 1.0f);
    setFloat(worldProgramID,"pointLights[1].linear", 0.027);
    setFloat(worldProgramID,"pointLights[1].quadratic", 0.0028);

    // point light 3
    setVec3(worldProgramID,"pointLights[2].position", pointLightPositions[2]);
    setVec3(worldProgramID,"pointLights[2].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    setVec3(worldProgramID,"pointLights[2].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    setVec3(worldProgramID,"pointLights[2].specular", glm::vec3(1.0f, 1.0f, 1.0f));
    setFloat(worldProgramID,"pointLights[2].constant", 1.0f);
    setFloat(worldProgramID,"pointLights[2].linear", 0.027);
    setFloat(worldProgramID,"pointLights[2].quadratic", 0.0028);
    // point light 4
    setVec3(worldProgramID,"pointLights[3].position", pointLightPositions[3]);
    setVec3(worldProgramID,"pointLights[3].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
    setVec3(worldProgramID,"pointLights[3].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    setVec3(worldProgramID,"pointLights[3].specular", glm::vec3(1.0f, 1.0f, 1.0f));
    setFloat(worldProgramID,"pointLights[3].constant", 1.0f);
    setFloat(worldProgramID,"pointLights[3].linear", 0.027);
    setFloat(worldProgramID,"pointLights[3].quadratic", 0.0028);
     //spotLight
    setVec3(worldProgramID,"spotLight.position", viewPos);
    setVec3(worldProgramID,"spotLight.direction",cammera_front );
    setVec3(worldProgramID,"spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
    setVec3(worldProgramID,"spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
    setVec3(worldProgramID,"spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    setFloat(worldProgramID,"spotLight.constant", 1.0f);
    setFloat(worldProgramID,"spotLight.linear", 0.0014);
    setFloat(worldProgramID,"spotLight.quadratic", 0.000007);
    setFloat(worldProgramID,"spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    setFloat(worldProgramID,"spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

}

void Window::drawModelNanosui()
{
    for(unsigned int i = 0; i < meshesNanosuit.size(); i++)
        meshesNanosuit[i].Draw(worldProgramID);

}
void Window::drawModelIronMan()
{
    for(unsigned int i = 0; i < meshesIronMan.size(); i++)
        meshesIronMan[i].Draw(worldProgramID);

}

void Window::loadModel(string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
    directoryNanosuit = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}
void Window::loadModelIronMan(string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
    directoryIranMan = path.substr(0, path.find_last_of('/'));

    processNodeIronMan(scene->mRootNode, scene);
}

void Window::processNode(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshesNanosuit.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}


void Window::processNodeIronMan(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshesIronMan.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNodeIronMan(node->mChildren[i], scene);
    }
}



Mesh Window::processMesh(aiMesh *mesh, const aiScene *scene)
{
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // Walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;
        // texture coordinates
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        // tangent
        if(mesh->mTangents)
        {
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
        }
        vertex.Tangent = vector;

        // bitangent
        if (mesh->mBitangents) {
                   vector.x = mesh->mBitangents[i].x;
             vector.y = mesh->mBitangents[i].y;
             vector.z = mesh->mBitangents[i].z;
        }
 
        vertex.Bitangent = vector;
        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    
    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<Texture> Window::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded_Nanosuit.size(); j++)
        {
            if(std::strcmp(textures_loaded_Nanosuit[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded_Nanosuit[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directoryNanosuit);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded_Nanosuit.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return textures;
}


vector<Texture> Window::loadMaterialTexturesIronMan(aiMaterial *mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded_IronMan.size(); j++)
        {
            if(std::strcmp(textures_loaded_IronMan[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded_IronMan[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directoryNanosuit);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded_IronMan.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return textures;
}


GLuint Window::TextureFromFile(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents=0;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



Window::~Window()
{

	// Cleanup VBO

    glDeleteBuffers(1, &cube_vertexbuffer);
    glDeleteBuffers(1, &tetrahedron_vertexbuffer);
	glDeleteBuffers(1, &light_vertexbuffer);
    glDeleteBuffers(1, &octahedron_vertexbuffer);
    glDeleteBuffers(1, &dodecahedron_vertexbuffer);
    glDeleteBuffers(1, &icosahedron_vertexbuffer);
    glDeleteBuffers(1, &house_vertexbuffer);
    glDeleteBuffers(1, &sphere_vertexbuffer);
    glDeleteBuffers(1, &house_norms_vertexbuffer);
    glDeleteBuffers(1, &icosahedron_norms_buffer);
    glDeleteBuffers(1, &octahedron_norm_buffer);
    glDeleteBuffers(1, &tetrahedron_norms_buffer);
    glDeleteBuffers(1, &cube_norms_buffer);
    glDeleteBuffers(1, &dodecahedron_norms_buffer);
    glDeleteBuffers(1, &sphere_norms_buffer);
    
//    glDeleteBuffers(1, &helicopter_norms_buffer);
//    glDeleteBuffers(1, &helicopter_vertexbuffer);

	glDeleteVertexArrays(1, &worldVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteProgram(worldProgramID);
	glDeleteProgram(lightProgramID);
    glDeleteProgram(phongProgramID);
    glDeleteProgram(gouraudProgramID);
    glDeleteProgram(modelProgramID);

    // Close OpenGL window and terminate GLFW
	glfwTerminate();
}
