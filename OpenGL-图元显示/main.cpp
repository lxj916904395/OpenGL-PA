//
//  main.cpp
//  OpenGL-图元显示
//
//  Created by zhongding on 2018/11/29.
//

#include "GLTools.h"
#include "GLFrustum.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

// 各种需要的类
GLShaderManager        shaderManager;
GLMatrixStack        modelViewMatrix;
GLMatrixStack        projectionMatrix;
GLFrame                cameraFrame;
GLFrame             objectFrame;

//投影矩阵
GLFrustum            viewFrustum;

//几何变换的管道
GLGeometryTransform    transformPipeline;
M3DMatrix44f        shadowMatrix;

//容器类（7种不同的图元对应7种容器对象）
GLBatch                pointBatch;
GLBatch                lineBatch;
GLBatch                lineStripBatch;
GLBatch                lineLoopBatch;
GLBatch                triangleBatch;
GLBatch             triangleStripBatch;
GLBatch             triangleFanBatch;


// 跟踪效果步骤
int nStep = 0;

GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
// 窗口已更改大小，或刚刚创建。无论哪种情况，我们都需要
// 使用窗口维度设置视口和投影矩阵.
void ChangeSize(int w, int h){
    
    glViewport(0, 0, w, h);
    
    //创建投影矩阵，并将它载入投影矩阵堆栈中
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //调用顶部载入单元矩阵
    modelViewMatrix.LoadIdentity();
}


//渲染图形
void RenderScene(){
    //清除颜色缓存去、深度缓冲区、模板缓冲区
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    //压栈
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    
    //矩阵乘以矩阵堆栈的顶部矩阵，相乘的结果随后简存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mCamera);
    
    M3DMatrix44f mObjectFrame;
    //只要使用 GetMatrix 函数就可以获取矩阵堆栈顶部的值，这个函数可以进行2次重载。用来使用GLShaderManager 的使用。或者是获取顶部矩阵的顶点副本数据
    objectFrame.GetMatrix(mObjectFrame);
    
    //矩阵乘以矩阵堆栈的顶部矩阵，相乘的结果随后简存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    /* GLShaderManager 中的Uniform 值——平面着色器
     参数1：平面着色器
     参数2：运行为几何图形变换指定一个 4 * 4变换矩阵
     --transformPipeline.GetModelViewProjectionMatrix() 获取的
     GetMatrix函数就可以获得矩阵堆栈顶部的值
     参数3：颜色值（黑色）
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    
    
    
    switch (nStep) {
        case 0:
            //点的大小
            glPointSize(4.0f);
            pointBatch.Draw();
            glPointSize(1.0f);
            
            break;
            
        default:
            break;
    }
    
    //还原到以前的模型视图矩阵（单位矩阵）
    modelViewMatrix.PopMatrix();
    
    // 进行缓冲区交换
    glutSwapBuffers();
}

//根据空格次数。切换不同的“窗口名称”
void KeyPressFunc(unsigned char key, int x, int y){
    
    if (key == 32){
        nStep++;
        if(nStep > 6) nStep = 0;
    }
    
    switch(nStep)
    {
        case 0:
            glutSetWindowTitle("点");
            break;
        case 1:
            glutSetWindowTitle("线");
            break;
        case 2:
            glutSetWindowTitle("线带");
            break;
        case 3:
            glutSetWindowTitle("线环");
            break;
        case 4:
            glutSetWindowTitle("三角形");
            break;
        case 5:
            glutSetWindowTitle("三角形金字塔");
            break;
        case 6:
            glutSetWindowTitle("三角形扇");
            break;
    }
    //提交渲染
    glutPostRedisplay();
    
}

//特殊键位处理（上、下、左、右移动）
void SpecialKeys(int key, int x, int y){
    //移动物体的位置有两种方法
    //1、改变物体的坐标
    //2、改变世界坐标系
    
    //围绕x轴旋转
    
    if (key == GLUT_KEY_UP)
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1, 0, 0);
    
    if (key == GLUT_KEY_DOWN)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1, 0, 0);
    
    //围绕y轴旋转
    
    if(key == GLUT_KEY_LEFT)
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    
    glutPostRedisplay();
}


void Setup(){
    
    //窗口背景色
    glClearColor(0.5f, 0.5f, 0.3f, 1.0);
    //初始化着色器管理器
    shaderManager.InitializeStockShaders();
    
    //开启深度测试
    glEnable(GL_DEPTH_TEST);
    
    //设置变换管线以使用两个矩阵堆栈
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
    //图元到窗口在z轴上的距离
    cameraFrame.MoveForward(-30.f);
    
    
    /*
     常见函数：
     void GLBatch::Begin(GLenum primitive,GLuint nVerts,GLuint nTextureUnits = 0);
     参数1：表示使用的图元
     参数2：顶点数
     参数3：纹理坐标（可选）
     
     //负责顶点坐标
     void GLBatch::CopyVertexData3f(GLFloat *vNorms);
     
     //结束，表示已经完成数据复制工作
     void GLBatch::End(void);
     
     */
    
    //定义一些点，类似佛罗里达州的形状。
    GLfloat vCoast[24][3] = {
        {2.80, 1.20, 0.0 }, {2.0,  1.20, 0.0 },
        {2.0,  1.08, 0.0 },  {2.0,  1.08, 0.0 },
        {0.0,  0.80, 0.0 },  {-.32, 0.40, 0.0 },
        {-.48, 0.2, 0.0 },   {-.40, 0.0, 0.0 },
        {-.60, -.40, 0.0 },  {-.80, -.80, 0.0 },
        {-.80, -1.4, 0.0 },  {-.40, -1.60, 0.0 },
        {0.0, -1.20, 0.0 },  { .2, -.80, 0.0 },
        {.48, -.40, 0.0 },   {.52, -.20, 0.0 },
        {.48,  .20, 0.0 },   {.80,  .40, 0.0 },
        {1.20, .80, 0.0 },   {1.60, .60, 0.0 },
        {2.0, .60, 0.0 },    {2.2, .80, 0.0 },
        {2.40, 1.0, 0.0 },   {2.80, 1.0, 0.0 }};
    
    pointBatch.Begin(GLT_ATTRIBUTE_VERTEX, 24);
    pointBatch.CopyVertexData3f(vCoast);
    pointBatch.End();
}


int main(int argc , char* argv[]){
//    初始化工作区间
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);

    //申请一个双缓存区、颜色缓存区、深度缓存区、模板缓存区
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //屏幕尺寸
    glutInitWindowSize(800, 600);
    
    //创建窗口
    glutCreateWindow("点图元");
    
    //注册回调函数（改变尺寸）
    glutReshapeFunc(ChangeSize);
    //点击空格时，调用的函数
    glutKeyboardFunc(KeyPressFunc);
    //特殊键位函数（上下左右）
    glutSpecialFunc(SpecialKeys);
    //显示函数
    glutDisplayFunc(RenderScene);
    
    GLenum err = glewInit();
    if (GLEW_OK != err){
        
        return 1;
    }
    
    Setup();
    
    //runloop运行循环
    glutMainLoop();
    
    return 0;
}

