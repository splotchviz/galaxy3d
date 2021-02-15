#include "Base.h"

int main() {
        VulkanExample *vulkanExample = new VulkanExample();
        vulkanExample->Launch();

        delete(vulkanExample);
        return 0;
}

