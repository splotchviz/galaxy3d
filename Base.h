

#ifndef __BaseApp_h
#define __BaseApp_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <array>
#include <iostream>
#include <algorithm>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "VulkanTools.h"
#include "VulkanBuffer.h"


#include "kernel/colour.h"
#include "cxxsupport/vec3.h"
#include "kernel/colourmap.h"
#include <fstream>
#include <limits.h>

#include "reader/reader.h"
#include "fitsReader.h"
#include <iostream>
#include <fstream>




struct particle
{
    float x, y, z, r;
    particle(float x1, float y1, float z1, float r1) {
        x = x1;
        y = y1;
        z = z1;
        r = r1;
    };
    particle() {}
};


struct SpaceObj {
    float position[4]={0,0,0,0};
    float color[4]={0,0,0,0};
        //float r;
        SpaceObj(float p1,float p2,float p3,float r1,float c1,float c2,float c3, float c4)
        {
            position[0]=p1;
            color[0]=c1;
            position[1]=p2;
            color[1]=c2;
            position[2]=p3;
            color[2]=c3;
            position[3]=r1;
            color[3]=c4;
          //  this->r=r1;

        };
        SpaceObj(){};
};


#define DEBUG (!NDEBUG)




class VulkanExample
{
public:
//Particle data
    std::vector<SpaceObj> vertices;
    std::vector<particle_sim> particle_data;



    uint32_t numParticles;
    particle p_min;
    particle p_max;

        VkInstance instance;


        VkPipelineCache pipelineCache;
        VkQueue queue;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        std::vector<VkShaderModule> shaderModules;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexMemory;

        // Resources for the graphics part of the example
        // may be should be replaced by Graph data in beginning
        struct {
            //conventional for pipeline
            uint32_t queueFamilyIndex;					// Used to check if compute and graphics queue families differ and require additional barriers
            VkDescriptorSetLayout descriptorSetLayout;	// Particle system rendering shader binding layout
            VkPipelineLayout pipelineLayout;			// Layout of the graphics pipeline
            VkPipeline pipeline;						// Particle rendering pipeline
            //VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
            vks::Buffer uniformBuffer;					// Contains scene matrices
            //can be used for textures that are sampled
            //currently replaced with SDF function
            //VkDescriptorSet descriptorSet;				// Particle system rendering shader bindings

            struct {
                glm::mat4 projection;
                glm::mat4 view;
                glm::vec2 screenDim;
            } ubo;
        } graphics;




        struct FrameBufferAttachment {
                VkImage image;
                VkDeviceMemory memory;
                VkImageView view;
        };


        VkFramebuffer framebuffer;
        FrameBufferAttachment colorAttachment, depthAttachment;
        VkRenderPass renderPass;





        VulkanExample();
        //is called on camera.updated
        //TODO: copying uniform buffer
        void updateGraphicsUniformBuffers();
        ~VulkanExample();

        /*
         * Helper functions for input parameters
        */

        void SetDims(int32_t width, int32_t height){
            m_width=width;
            m_height=height;
        }

        void SetFilename(std::string file){
            m_filename=file;
        }
        void Launch();
        void SaveFile(int i);
        void ReadFitsFile();
        void ReadParFile();
        void PrepaireVertices();



        /*
         * VULKAN FUNCTIONS
         */

        void CreateFrameBufferAttachements(VkFormat& colorFormat,VkFormat& depthFormat);
        void CreateRenderPass(VkFormat colorFormat,VkFormat depthFormat);
        void CreateCommandBuffer(int i, int j, int k);


private:

        //particles stuff
             int32_t m_width, m_height;
             std::string m_filename;
             std::string m_outputfilename;
             glm::vec3 m_center;
             glm::vec3 m_scale;
             std::string m_Name;

          //Vulkan stuff
             VkPhysicalDevice physicalDevice;
             VkDevice device;

             /*
              * Functions that probably will be unchanged
               */

             uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);
             VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer *buffer, VkDeviceMemory *memory, VkDeviceSize size, void *data = nullptr);

             void submitWork(VkCommandBuffer cmdBuffer, VkQueue queue);

             /*
              * Graphics related stuff
              */
             void SetGraphicsAndCommandPool();
             void PrepareParticlesBuffer();
             void DescribeBindShaderData(); //right now describes pushed matrix

             //TODO: will be added later
             VkDebugReportCallbackEXT debugReportCallback{};

             //Vulkan basic functions, probably will be unchanged
             void vkInit();
             void vkDeviceChoice();
             void EntireRasterGraphicsPipelineSet(VkPipelineShaderStageCreateInfo* shaderStages, uint32_t ShaderSize);//TODO: split on functions that will be unchanged

};




#endif
