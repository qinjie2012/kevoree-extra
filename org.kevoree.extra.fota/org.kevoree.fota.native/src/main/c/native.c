#include <jni.h>
#include <nativelib.h>
#include <stdio.h>
#include "fota.h"

 /**
  * Created by jed
  * User: jedartois@gmail.com
  * Date: 18/07/12
  * Time: 8:49
  */


JavaVM * g_vm;
jobject g_obj;
jmethodID g_mid;

unsigned char * file_intel_hex_array;



void events_fota(int evt)
{
    JNIEnv * g_env;
    int getEnvStat = (*g_vm)->GetEnv(g_vm,(void **)&g_env, JNI_VERSION_1_6);

    if (getEnvStat == JNI_EDETACHED)
    {
        if ((*g_vm)->AttachCurrentThread(g_vm,(void **) &g_env, NULL) != 0)
        {
                   printf("Failed to attach\n");
        }
    } else if (getEnvStat == JNI_OK)
    {
        //
    } else if (getEnvStat == JNI_EVERSION)
    {
        printf("GetEnv: version not supported\n");
    }

    (*g_env)->CallVoidMethod(g_env,g_obj, g_mid, evt);

    if ((*g_env)->ExceptionCheck(g_env)) {
        (*g_env)->ExceptionDescribe(g_env);
    }
    (*g_vm)->DetachCurrentThread(g_vm);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    // register events_fota
    register_FlashEvent(events_fota);
    g_vm = jvm;
    return JNI_VERSION_1_6;
}



JNIEXPORT jboolean JNICALL Java_org_kevoree_fota_Nativelib_register  (JNIEnv *env, jobject obj)
{
               // convert local to global reference
            g_obj = (*env)->NewGlobalRef(env,obj);
            // save refs for callback
            jclass g_clazz = (*env)->GetObjectClass(env,g_obj);

            if (g_clazz == NULL)
            {
                printf( "Failed to find class \n");
            }

            g_mid = (*env)->GetMethodID(env,g_clazz, "dispatchEvent", "(I)V");
            if (g_mid == NULL)
             {
              printf( "Unable to get method ref");

             }
             return (jboolean)0;
}

JNIEXPORT void JNICALL Java_org_kevoree_fota_Nativelib_close_1flash(JNIEnv * env, jobject obj)
{
   close_flash();
}


JNIEXPORT jint JNICALL Java_org_kevoree_fota_Nativelib_write_1on_1the_1air_1program  (JNIEnv *env, jobject obj, jstring device, jint target, jstring path)
{
   struct file_buffer_t *file;
   // convert
  const char *n_device = (*env)->GetStringUTFChars(env, device, 0);
  const char *filename = (*env)->GetStringUTFChars(env, path, 0);
  //printf("Openning %s %s \n",n_device,filename);
  file = readFile ((const char*)filename);
  if (!file)
  {
     return FAIL_OPEN_FILE;
  }
    fprintf(stderr,"hex size %d octets\n",file->length);
  int rt =write_on_the_air_program(n_device,target,file->length,file->data);


   return  rt;
}



