#include "Base.h"



        uint32_t VulkanExample::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) {
                VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
                vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
                for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
                        if ((typeBits & 1) == 1) {
                                if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                                        return i;
                                }
                        }
                        typeBits >>= 1;
                }
                return 0;
        }

        VkResult VulkanExample::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer *buffer, VkDeviceMemory *memory, VkDeviceSize size, void *data)
        {
                // Create the buffer handle
                VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
                bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer));

                // Create the memory backing up the buffer handle
                VkMemoryRequirements memReqs;
                VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
                vkGetBufferMemoryRequirements(device, *buffer, &memReqs);
                memAlloc.allocationSize = memReqs.size;
                memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, memoryPropertyFlags);
                VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, memory));

                if (data != nullptr) {
                        void *mapped;
                        VK_CHECK_RESULT(vkMapMemory(device, *memory, 0, size, 0, &mapped));
                        memcpy(mapped, data, size);
                        vkUnmapMemory(device, *memory);
                }

                VK_CHECK_RESULT(vkBindBufferMemory(device, *buffer, *memory, 0));

                return VK_SUCCESS;
        }

        /*
                Submit command buffer to a queue and wait for fence until queue operations have been finished
        */
          void VulkanExample::submitWork(VkCommandBuffer cmdBuffer, VkQueue queue)
        {
                VkSubmitInfo submitInfo = vks::initializers::submitInfo();
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &cmdBuffer;
                VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo();
                VkFence fence;
                VK_CHECK_RESULT(vkCreateFence(device, &fenceInfo, nullptr, &fence));
                VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
                VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
                vkDestroyFence(device, fence, nullptr);
        }


        VulkanExample::VulkanExample()
        {

            m_width = 1024;
            m_height = 1024;
            m_Name="VulkanExample";
            m_filename="LVHIS027.na.icln.fits";//"snap092.par"; LVHIS082 -is problematic with TF todo
        }

        void VulkanExample::CreateFrameBufferAttachements(VkFormat& colorFormat,VkFormat& depthFormat)
        {

            //helping function from SaschaWillems codes
        vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);

                // Color attachment
                VkImageCreateInfo image = vks::initializers::imageCreateInfo();
                image.imageType = VK_IMAGE_TYPE_2D;
                image.format = colorFormat;
                image.extent.width = m_width;
                image.extent.height = m_height;
                image.extent.depth = 1;
                image.mipLevels = 1;
                image.arrayLayers = 1;
                image.samples = VK_SAMPLE_COUNT_1_BIT;
                image.tiling = VK_IMAGE_TILING_OPTIMAL;
                image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

                VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
                VkMemoryRequirements memReqs;

                VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &colorAttachment.image));
                vkGetImageMemoryRequirements(device, colorAttachment.image, &memReqs);
                memAlloc.allocationSize = memReqs.size;
                memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &colorAttachment.memory));
                VK_CHECK_RESULT(vkBindImageMemory(device, colorAttachment.image, colorAttachment.memory, 0));

                VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
                colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
                colorImageView.format = colorFormat;
                colorImageView.subresourceRange = {};
                colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                colorImageView.subresourceRange.baseMipLevel = 0;
                colorImageView.subresourceRange.levelCount = 1;
                colorImageView.subresourceRange.baseArrayLayer = 0;
                colorImageView.subresourceRange.layerCount = 1;
                colorImageView.image = colorAttachment.image;
                VK_CHECK_RESULT(vkCreateImageView(device, &colorImageView, nullptr, &colorAttachment.view));

                // Depth stencil attachment
                image.format = depthFormat;
                image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

                VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &depthAttachment.image));
                vkGetImageMemoryRequirements(device, depthAttachment.image, &memReqs);
                memAlloc.allocationSize = memReqs.size;
                memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &depthAttachment.memory));
                VK_CHECK_RESULT(vkBindImageMemory(device, depthAttachment.image, depthAttachment.memory, 0));

                VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
                depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
                depthStencilView.format = depthFormat;
                depthStencilView.flags = 0;
                depthStencilView.subresourceRange = {};
                depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                depthStencilView.subresourceRange.baseMipLevel = 0;
                depthStencilView.subresourceRange.levelCount = 1;
                depthStencilView.subresourceRange.baseArrayLayer = 0;
                depthStencilView.subresourceRange.layerCount = 1;
                depthStencilView.image = depthAttachment.image;
                VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &depthAttachment.view));


        }

        void VulkanExample::CreateCommandBuffer(int i, int j, int k)
        {
            VkCommandBuffer commandBuffer;
            VkCommandBufferAllocateInfo cmdBufAllocateInfo =
                    vks::initializers::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
            VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &commandBuffer));

            VkCommandBufferBeginInfo cmdBufInfo =
                    vks::initializers::commandBufferBeginInfo();

            VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));

            VkClearValue clearValues[2];
            clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            clearValues[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderArea.extent.width = m_width;
            renderPassBeginInfo.renderArea.extent.height = m_height;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValues;
            renderPassBeginInfo.renderPass = renderPass;
            renderPassBeginInfo.framebuffer = framebuffer;

            vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = {};
            viewport.height = (float)m_height;
            viewport.width = (float)m_width;
            viewport.minDepth = (float)0.0f;
            viewport.maxDepth = (float)1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            // Update dynamic scissor state
            VkRect2D scissor = {};
            scissor.extent.width = m_width;
            scissor.extent.height = m_height;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);

            // Render scene
            VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
            //vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);




            //for (auto v : pos)
            auto v=glm::vec3(0.0f, 0.0f, -1.2f/(m_scale.x*50));
            //Compute center
           // glm::vec3 v=glm::vec3((p_min.x+p_max.x)/2, (p_min.y+p_max.y)/2,(p_min.z-14.0f));
            //std::cout<<v.x<<" center"<<std::endl;
            {
                    glm::mat4 mvpMatrix = glm::perspective(glm::radians(60.0f), (float)m_width / (float)m_height, 0.1f, 256.0f) * glm::translate(glm::mat4(1.0f), v);
                    //scale
                    mvpMatrix = glm::scale(mvpMatrix, m_scale);
                    //rotation
                    glm::mat4 rotM = glm::mat4(1.0f);
                    glm::vec3 rotation=glm::vec3(10*i,10*j,10*k);///50,170,50);//(50,170,50);//135,-5,-15);//

                    rotM = glm::rotate(rotM, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                    rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                    rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

                    mvpMatrix*=rotM;

                    //camera matrix

               /*
               camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 512.0f);
                camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
                camera.setTranslation(glm::vec3(0.0f, 0.0f, -4.0f));*/
                //currently as constant
                   // graphics.ubo.projection = camera.matrices.perspective;
                   // graphics.ubo.view = camera.matrices.view;
                   //graphics.ubo.cam=mvpMatrix;

                    vkCmdPushConstants(commandBuffer, graphics.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvpMatrix), &mvpMatrix);
                   vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
            }

            vkCmdEndRenderPass(commandBuffer);

            VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

            submitWork(commandBuffer, queue);

            vkDeviceWaitIdle(device);
    }


        void VulkanExample:: ReadParFile()
        {
            paramfile params(m_filename, false);
            printf("Start \n" );
            int interpol_mode;
            double boxsize;
            //params.

            // only used if interpol_mode>0
            std::vector<particle_sim> p1, p2;
            std::vector<MyIDType> id1, id2;
            std::vector<MyIDType> idx1, idx2;

            int snr1_now, snr2_now;

            // buffers to hold the times relevant to the *currently loaded snapshots*
            double time1, time2;
            double redshift1, redshift2;

            // only used if interpol_mode>1
            std::vector<vec3f> vel1, vel2;

            // only used if the same particles are used for more than one scene

            int simtype = params.find<int>("simtype");
            int spacing = params.find<double>("snapshot_spacing", 1);
            double fidx = params.find<double>("fidx", 0);
            int snr1_guess = int(fidx / spacing)*spacing, snr2_guess = snr1_guess + spacing;
            int snr1 = params.find<int>("snapshot_base1", snr1_guess);
            int snr2 = params.find<int>("snapshot_base2", snr2_guess);
            double frac = params.find<float>("frac", (fidx - snr1) / (snr2 - snr1));


            //gadget_reader(params, interpol_mode, particle_data, id1, vel1, snr1, time1, boxsize);
            //   std::cout<<particle_data.size()<<" r= "<<particle_data[10].r<<" I= "<<particle_data[10].I<<std::endl;

           /*    std::cout<<"binary file reading"<<std::endl;

               paramfile params2("reduced.par", false);
               bin_reader_tab(params2, particle_data);
               std::cout << "particles " << particle_data.size() << " \n";
               std::cout << "part1 " << particle_data[0].x << "," << particle_data[0].y << "," << particle_data[0].z << "  " << particle_data[0].r << "  " << particle_data[0].I << "  " << particle_data[0].e.g <<" \n";

   */
        }
        void VulkanExample::ReadFitsFile()
        {
            fitsReader fits;
            fits.is3D=true;
            fits.SetFileName(m_filename);//LVHIS001.na.icln.fits");//LVHIS027.na.icln.fits");//
            fits.ReadFits(particle_data);

            //float max=fits.GetMax();
           // float min=fits.GetMin();


        }
        void VulkanExample::CreateRenderPass(VkFormat colorFormat,VkFormat depthFormat)
        {
                std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
                // Color attachment
                attchmentDescriptions[0].format = colorFormat;
                attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
                attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                // Depth attachment
                attchmentDescriptions[1].format = depthFormat;
                attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
                attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
                VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

                VkSubpassDescription subpassDescription = {};
                subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpassDescription.colorAttachmentCount = 1;
                subpassDescription.pColorAttachments = &colorReference;
                subpassDescription.pDepthStencilAttachment = &depthReference;

                // Use subpass dependencies for layout transitions
                std::array<VkSubpassDependency, 2> dependencies;

                dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                dependencies[0].dstSubpass = 0;
                dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

                dependencies[1].srcSubpass = 0;
                dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

                // Create the actual renderpass
                VkRenderPassCreateInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
                renderPassInfo.pAttachments = attchmentDescriptions.data();
                renderPassInfo.subpassCount = 1;
                renderPassInfo.pSubpasses = &subpassDescription;
                renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
                renderPassInfo.pDependencies = dependencies.data();
                VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

                VkImageView attachments[2];
                attachments[0] = colorAttachment.view;
                attachments[1] = depthAttachment.view;

                VkFramebufferCreateInfo framebufferCreateInfo = vks::initializers::framebufferCreateInfo();
                framebufferCreateInfo.renderPass = renderPass;
                framebufferCreateInfo.attachmentCount = 2;
                framebufferCreateInfo.pAttachments = attachments;
                framebufferCreateInfo.width = m_width;
                framebufferCreateInfo.height = m_height;
                framebufferCreateInfo.layers = 1;
                VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffer));
        }
        void VulkanExample::PrepaireVertices()
        {

            int num = 0;
            int skip_rate = 1;

            p_min = particle(particle_data[0].x, particle_data[0].y, particle_data[0].z, particle_data[0].r);
            p_max = particle(particle_data[0].x, particle_data[0].y, particle_data[0].z, particle_data[0].r);

            for (int i = 0; i < particle_data.size(); i++)
            {
                //if (particle_data[i].r > 0.0000001) //gas
                {
                    particle pp = particle(particle_data[i].x, particle_data[i].y, particle_data[i].z, particle_data[i].r);

                    num++;
                    p_min.x = std::min(p_min.x, (float)pp.x);
                    p_min.y = std::min(p_min.y, (float)pp.y);
                    p_min.z = std::min(p_min.z, (float)pp.z);
                    p_min.r = std::min(p_min.r, (float)pp.r);

                    if (pp.x > p_max.x) p_max.x = pp.x;
                    if (pp.y > p_max.y) p_max.y = pp.y;
                    if (pp.z > p_max.z) p_max.z = pp.z;
                    if (pp.r > p_max.r) p_max.r = pp.r;
                    // myfile << "part2 " << particle_data[i].x << "," << particle_data[i].y << "," << particle_data[i].z << "  " << particle_data[i].r << " \n";
                }
            }

           // std::cout<<p_min.z<<" min "<<p_max.z<<" max"<<std::endl;



            float x_scale = p_max.x - p_min.x;
            float y_scale = p_max.y - p_min.y;
            float z_scale = p_max.z - p_min.z;
            float r_scale = p_max.r - p_min.r;

            //TODO: scaling for fits
            /// r_scale * sc - carefull
            glm::vec3 c=glm::vec3((p_min.x+p_max.x)/2, (p_min.y+p_max.y)/2,(p_min.z+p_max.z)/2);

            m_center=glm::vec3((p_min.x+p_max.x)/2, (p_min.y+p_max.y)/2,(p_min.z+p_max.z)/2);

            float sc=2.0;//10.2; //for camera in 20
            float scale=fmax(x_scale,y_scale);
            scale=fmax(scale,z_scale);

            scale=sc/scale;
            m_scale=glm::vec3(scale,scale,scale);


            for (int i = 0; i < particle_data.size(); i++)
            {
                //if (particle_data[i].r > 0.0001) //gas
                {
                    SpaceObj s(particle_data[i].x-c.x, particle_data[i].y-c.y, particle_data[i].z-c.z, particle_data[i].r /*/ r_scale * sc*/, particle_data[i].e.r, particle_data[i].e.g, particle_data[i].e.b, particle_data[i].I);

                    vertices.push_back(s);
                }
            }


            vertices.resize(particle_data.size());
            printf("%lu number of vertices \n",vertices.size());

        }



        void VulkanExample::vkInit()
        {





            VkApplicationInfo appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = m_Name.c_str();
            appInfo.pEngineName = m_Name.c_str();
            appInfo.apiVersion = VK_API_VERSION_1_0;

            /*
                    Vulkan instance creation
                    Here we do not use surface extensions
                    TODO: reconsider for GFLW switch
            */
            VkInstanceCreateInfo instanceCreateInfo = {};
            instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceCreateInfo.pApplicationInfo = &appInfo;



/*
        //TODO: validation layers, different GPU cards and etc
*/
            VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));


        }

        void VulkanExample::vkDeviceChoice()
        {
            /*
                    Vulkan device creation
            */
        uint32_t deviceCount = 0;
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));
        physicalDevice = physicalDevices[0];

        bool discrete=false;


        for (const auto& device : physicalDevices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);
            defInfo.deviceType=props.deviceType; //save device type

            // Determine the type of the physical device
            if (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                printf("a discrete GPU. (ideal)\n") ;
                discrete=true;
                physicalDevice=device;

            }
            else if (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {

                 printf("an integrated GPU\n") ;
                 if(!discrete)
                     physicalDevice=device;
            }
            else
            {
                printf("something else\n") ;
            }


        }

        //check extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
         printf("Full list of available extensions:\n");

        for (const auto& extension : extensions) {
             printf("%s \n",extension.extensionName);
            if (std::string(extension.extensionName).compare("VK_KHR_surface") != 0)
                 defInfo.interactive=true;
             if (extension.extensionName=="VK_KHR_ray_tracing")
                 defInfo.ray_tracing=true;

        }

        }

        void VulkanExample::SetGraphicsAndCommandPool()
        {

        VkPhysicalDeviceProperties deviceProperties={};
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
       // std::cout<<"GPU: "<< deviceProperties.deviceName<<std::endl;

        //  single graphics queue as far
        const float defaultQueuePriority(0.0f);
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
                if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        graphics.queueFamilyIndex = i;
                        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                        queueCreateInfo.queueFamilyIndex = i;
                        queueCreateInfo.queueCount = 1;
                        queueCreateInfo.pQueuePriorities = &defaultQueuePriority;
                        break;
                }
        }
        // Create logical device
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

        // Get a graphics queue
        vkGetDeviceQueue(device, graphics.queueFamilyIndex, 0, &queue);

        // Command pool
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = graphics.queueFamilyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &commandPool));

        }


        /*
                Prepare particles buffer
        */

        void VulkanExample::PrepareParticlesBuffer()

        {

                //std::vector<uint32_t> indices = { 0, 1, 2 };

                const VkDeviceSize vertexBufferSize = vertices.size() * sizeof(SpaceObj);


                VkBuffer stagingBuffer;
                VkDeviceMemory stagingMemory;

                // Command buffer for copy commands
                VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
                VkCommandBuffer copyCmd;
                VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &copyCmd));
                VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

                // Copy input data to buffers
                {
                        // Vertices
                        createBuffer(
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                &stagingBuffer,
                                &stagingMemory,
                                vertexBufferSize,
                                vertices.data());

                        createBuffer(
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                &vertexBuffer,
                                &vertexMemory,
                                vertexBufferSize);

                        VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
                        VkBufferCopy copyRegion = {};
                        copyRegion.size = vertexBufferSize;
                        vkCmdCopyBuffer(copyCmd, stagingBuffer, vertexBuffer, 1, &copyRegion);
                        VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

                        submitWork(copyCmd, queue);

                        vkDestroyBuffer(device, stagingBuffer, nullptr);
                        vkFreeMemory(device, stagingMemory, nullptr);


                }
        }

        void VulkanExample::DescribeBindShaderData()
        {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
        VkDescriptorSetLayoutCreateInfo descriptorLayout =
                vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &graphics.descriptorSetLayout));


        /* Bind data with uniform buffer alternative and save it to graphics info
         Advantage: all associated data is stored in graphics struct
        */
      /*  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
        setLayoutBindings = {
                  vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 2),
        };

        VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(
                setLayoutBindings.data(),
                static_cast<uint32_t>(setLayoutBindings.size()));

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(
                &graphics.descriptorSetLayout,
                1);

        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &graphics.pipelineLayout));
*/
        /* ----end ofBind data*/

        //bind data simple
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
                vks::initializers::pipelineLayoutCreateInfo(nullptr, 0);

        // MVP via push constant block
        VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;


        //Finished with layout with vars and now pipelineCreateInfo with vertices and etc.

        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &graphics.pipelineLayout));
            //------end of bind data simple
        }

        /*
                Graphics pipeline itself: shaders, binding data for shaders and pipeline charecteristics.
        */

        void VulkanExample::EntireRasterGraphicsPipelineSet(VkPipelineShaderStageCreateInfo* shaderStages,uint32_t ShaderSize)
        {
                DescribeBindShaderData();

                VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
                pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
                VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

                // Create pipeline for particle based
                //key features:point topology, blending, depth
                VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
                        vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, VK_FALSE);

                VkPipelineRasterizationStateCreateInfo rasterizationState =
                        vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);

                VkPipelineColorBlendAttachmentState blendAttachmentState =
                        vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

                VkPipelineColorBlendStateCreateInfo colorBlendState =
                        vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

                // Additive blending
                blendAttachmentState.colorWriteMask = 0xF;
                blendAttachmentState.blendEnable = VK_TRUE;
                blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
                blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
                blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
                blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;


                VkPipelineDepthStencilStateCreateInfo depthStencilState =
                    vks::initializers::pipelineDepthStencilStateCreateInfo(
                        VK_FALSE,
                        VK_FALSE,
                        VK_COMPARE_OP_ALWAYS);

                VkPipelineViewportStateCreateInfo viewportState =
                        vks::initializers::pipelineViewportStateCreateInfo(1, 1);

                VkPipelineMultisampleStateCreateInfo multisampleState =
                        vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

                std::vector<VkDynamicState> dynamicStateEnables = {
                        VK_DYNAMIC_STATE_VIEWPORT,
                        VK_DYNAMIC_STATE_SCISSOR
                };
                VkPipelineDynamicStateCreateInfo dynamicState =
                        vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

                VkGraphicsPipelineCreateInfo pipelineCreateInfo =
                        vks::initializers::pipelineCreateInfo(graphics.pipelineLayout, renderPass);



                pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
                pipelineCreateInfo.pRasterizationState = &rasterizationState;
                pipelineCreateInfo.pColorBlendState = &colorBlendState;
                pipelineCreateInfo.pMultisampleState = &multisampleState;
                pipelineCreateInfo.pViewportState = &viewportState;
                pipelineCreateInfo.pDepthStencilState = &depthStencilState;
                pipelineCreateInfo.pDynamicState = &dynamicState;

                pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderModules.size());
                pipelineCreateInfo.pStages = shaderStages;





                // Vertex bindings an attributes
                // Binding description
                std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
                        vks::initializers::vertexInputBindingDescription(0, sizeof(SpaceObj), VK_VERTEX_INPUT_RATE_VERTEX),
                };

                // Attribute descriptions
                std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
                        vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0),					// Position
                        vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4),	// Color
                };

                VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
                vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
                vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
                vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
                vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

                pipelineCreateInfo.pVertexInputState = &vertexInputState;


                VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphics.pipeline));
        }

        void VulkanExample::Launch()
        {

            int cur_scene;
            if(m_filename.substr(m_filename.find_last_of(".") + 1) == "fits")
                ReadFitsFile();
            else
                ReadParFile();

           //preprocess and prepaire vertices array
            PrepaireVertices();

            vkInit();
            vkDeviceChoice();

            SetGraphicsAndCommandPool(); //sets graphics for device

            PrepareParticlesBuffer(); //particles to buffer and submit work



                /*
                        Create framebuffer attachments
                */

                VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
                VkFormat depthFormat;

             CreateFrameBufferAttachements(colorFormat,depthFormat);
                /*
                        Create renderpass
                */
                CreateRenderPass(colorFormat,depthFormat);

                /*
                 * Set shaders
                 * */
                VkPipelineShaderStageCreateInfo shaderStages[2];


                shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
                shaderStages[0].pName = "main";
                shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shaderStages[1].pName = "main";

                shaderStages[0].module = vks::tools::loadShader("triangle.vert.spv", device);
                shaderStages[1].module = vks::tools::loadShader("triangle.frag.spv", device);

                //save to global variable links to files, may be further entire shaders
                shaderModules = { shaderStages[0].module, shaderStages[1].module };




                EntireRasterGraphicsPipelineSet( shaderStages,2);
                /*
                    Rendering with camera update (test)
                */
                for (int j=0;j<36*3+1;j++)
                {
                   if(j<37)
                CreateCommandBuffer(j,0,0);
                    else
                    {
                        if(j<=36*2)
                    CreateCommandBuffer(36,j-36,0);
                        else
                            CreateCommandBuffer(0,2*36,j-2*36);
                    }

                    //CreateCommandBuffer(0,0,j);

                /*
                   Test of writing several files
                */
               SaveFile(j);
                }
                vkQueueWaitIdle(queue);
        }

        //is called on camera.updated
        //TODO: copying uniform buffer
        void VulkanExample::updateGraphicsUniformBuffers()
        {
           // graphics.ubo.projection = camera.matrices.perspective;
          //  graphics.ubo.view = camera.matrices.view;
            graphics.ubo.screenDim = glm::vec2((float)m_width, (float)m_height);
            memcpy(graphics.uniformBuffer.mapped, &graphics.ubo, sizeof(graphics.ubo));
        }

        void VulkanExample::SaveFile(int i)
        {
            const char* imagedata;

                    // Create the linear tiled destination image to copy to and to read the memory from
                    VkImageCreateInfo imgCreateInfo(vks::initializers::imageCreateInfo());
                    imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                    imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
                    imgCreateInfo.extent.width = m_width;
                    imgCreateInfo.extent.height = m_height;
                    imgCreateInfo.extent.depth = 1;
                    imgCreateInfo.arrayLayers = 1;
                    imgCreateInfo.mipLevels = 1;
                    imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
                    imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                    // Create the image
                    VkImage dstImage;
                    VK_CHECK_RESULT(vkCreateImage(device, &imgCreateInfo, nullptr, &dstImage));
                    // Create memory to back up the image
                    VkMemoryRequirements memRequirements;
                    VkMemoryAllocateInfo memAllocInfo(vks::initializers::memoryAllocateInfo());
                    VkDeviceMemory dstImageMemory;
                    vkGetImageMemoryRequirements(device, dstImage, &memRequirements);
                    memAllocInfo.allocationSize = memRequirements.size;
                    // Memory must be host visible to copy from
                    memAllocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                    VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &dstImageMemory));
                    VK_CHECK_RESULT(vkBindImageMemory(device, dstImage, dstImageMemory, 0));

                    // Do the actual blit from the offscreen image to our host visible destination image
                    VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
                    VkCommandBuffer copyCmd;
                    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &copyCmd));
                    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
                    VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));

                    // Transition destination image to transfer destination layout
                    vks::tools::insertImageMemoryBarrier(
                            copyCmd,
                            dstImage,
                            0,
                            VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

                    // colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

                    VkImageCopy imageCopyRegion{};
                    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageCopyRegion.srcSubresource.layerCount = 1;
                    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    imageCopyRegion.dstSubresource.layerCount = 1;
                    imageCopyRegion.extent.width = m_width;
                    imageCopyRegion.extent.height = m_height;
                    imageCopyRegion.extent.depth = 1;

                    vkCmdCopyImage(
                            copyCmd,
                            colorAttachment.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &imageCopyRegion);

                    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
                    vks::tools::insertImageMemoryBarrier(
                            copyCmd,
                            dstImage,
                            VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_ACCESS_MEMORY_READ_BIT,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_GENERAL,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

                    VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

                    submitWork(copyCmd, queue);

                    // Get layout of the image (including row pitch)
                    VkImageSubresource subResource{};
                    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    VkSubresourceLayout subResourceLayout;

                    vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

                    // Map image memory so we can start copying from it
                    vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
                    imagedata += subResourceLayout.offset;

            /*
                    Save host visible framebuffer image to disk (ppm format)
            */


                      char str[16];
                      if(i<10)
 sprintf(str, "%s%d%s","00", i,"_headless.ppm");
                      else {
                          if(i<100)
                               sprintf(str, "%s%d%s","0", i,"_headless.ppm");
                              else
                      sprintf(str, "%d%s", i,"_headless.ppm");
                      }

                    const char* filename = str;

                    std::ofstream file(filename, std::ios::out | std::ios::binary);

                    // ppm header
                    file << "P6\n" << m_width << "\n" << m_height << "\n" << 255 << "\n";

                    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
                    // Check if source is BGR and needs swizzle
                    std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
                    const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

                    // ppm binary pixel data
                    for (int32_t y = 0; y < m_height; y++) {
                            unsigned int *row = (unsigned int*)imagedata;
                            for (int32_t x = 0; x < m_width; x++) {
                                    if (colorSwizzle) {
                                            file.write((char*)row + 2, 1);
                                            file.write((char*)row + 1, 1);
                                            file.write((char*)row, 1);
                                    }
                                    else {
                                            file.write((char*)row, 3);
                                    }
                                    row++;
                            }
                            imagedata += subResourceLayout.rowPitch;
                    }
                    file.close();

                    printf("Framebuffer image saved to %s\n", filename);

                    // Clean up resources
                    vkUnmapMemory(device, dstImageMemory);
                    vkFreeMemory(device, dstImageMemory, nullptr);
                    vkDestroyImage(device, dstImage, nullptr);


        }

        VulkanExample::~VulkanExample()
        {
            //TODO: add some debug messages here

                vkDestroyBuffer(device, vertexBuffer, nullptr);
                vkFreeMemory(device, vertexMemory, nullptr);

                vkDestroyImageView(device, colorAttachment.view, nullptr);
                vkDestroyImage(device, colorAttachment.image, nullptr);
                vkFreeMemory(device, colorAttachment.memory, nullptr);
                vkDestroyImageView(device, depthAttachment.view, nullptr);
                vkDestroyImage(device, depthAttachment.image, nullptr);
                vkFreeMemory(device, depthAttachment.memory, nullptr);
                vkDestroyRenderPass(device, renderPass, nullptr);
                vkDestroyFramebuffer(device, framebuffer, nullptr);
                vkDestroyPipelineLayout(device, graphics.pipelineLayout, nullptr);
                vkDestroyDescriptorSetLayout(device, graphics.descriptorSetLayout, nullptr);
                vkDestroyPipeline(device, graphics.pipeline, nullptr);
                vkDestroyPipelineCache(device, pipelineCache, nullptr);
                vkDestroyCommandPool(device, commandPool, nullptr);
                for (auto shadermodule : shaderModules) {
                        vkDestroyShaderModule(device, shadermodule, nullptr);
                }
                vkDestroyDevice(device, nullptr);

                vkDestroyInstance(instance, nullptr);

        }

