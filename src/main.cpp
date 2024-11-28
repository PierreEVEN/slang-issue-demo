#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "slang-com-ptr.h"
#include "slang.h"


Slang::ComPtr<slang::IGlobalSession> global_session;
Slang::ComPtr<slang::ISession> session;
slang::IModule* module = nullptr;

std::unordered_map<std::string, bool> permutations;


std::string reflection_test()
{

    for (SlangInt32 ep_i = 0; ep_i < module->getDefinedEntryPointCount(); ++ep_i)
    {
        Slang::ComPtr<slang::IEntryPoint> entry_point;
        if (SLANG_FAILED(module->getDefinedEntryPoint(ep_i, entry_point.writeRef())))
            return "Failed to get entry point";

        std::vector<slang::IComponentType*> components;

        /*
        if (!permutations.keys().empty())
        {
            std::string generated_code_string;
            for (const auto& switch_keys : permutation.keys())
                generated_code_string += std::cout << "export static const bool " << switch_keys << " = " << (permutation.get(switch_keys) ? "true" : "false") << ";";

            // Load the specialization constant module from string.
            auto permutation_values_src = UnownedRawBlob::create(generated_code_string.c_str(), generated_code_string.size());
            components.emplace_back(session->loadModuleFromSource("permutation-values", "permutation-values.slang", permutation_values_src));
        }
		*/

        components.emplace_back(entry_point);
        Slang::ComPtr<slang::IComponentType> program;
        if (SLANG_FAILED(session->createCompositeComponentType(components.data(), components.size(), program.writeRef())))
            return "Failed to create stage program";

        Slang::ComPtr<slang::IComponentType> linkedProgram;
        Slang::ComPtr<slang::IBlob>          diagnostics;
        program->link(linkedProgram.writeRef(), diagnostics.writeRef());
        if (diagnostics)
            return static_cast<const char*>(diagnostics->getBufferPointer());

        Slang::ComPtr<slang::IBlob> kernel_blob;
        linkedProgram->getEntryPointCode(0, 0, kernel_blob.writeRef(), diagnostics.writeRef());

        std::vector<uint8_t> buffer(kernel_blob->getBufferSize());
        memcpy(buffer.data(), kernel_blob->getBufferPointer(), kernel_blob->getBufferSize());

        slang::IMetadata* metadata;
        linkedProgram->getEntryPointMetadata(0, 0, &metadata, diagnostics.writeRef());
        if (diagnostics)
            return static_cast<const char*>(diagnostics->getBufferPointer());

        slang::ProgramLayout* shaderReflection = linkedProgram->getLayout();
        for (unsigned par_i = 0; par_i < shaderReflection->getParameterCount(); par_i++)
        {
            bool                             b_is_used = true;
            slang::VariableLayoutReflection* parameter = shaderReflection->getParameterByIndex(par_i);
            metadata->isParameterLocationUsed(static_cast<SlangParameterCategory>(parameter->getCategory()), 0, parameter->getBindingIndex(), b_is_used);

            if (parameter->getCategory() == slang::PushConstantBuffer && !b_is_used)
            {
                if (entry_point->getLayout()->getEntryPointByIndex(0)->getStage() == SLANG_STAGE_VERTEX)
                {
                    return "UNUSED PC ISSUE !!!!\n";
                }
            }

            if (b_is_used)
            {
                    return "Unhandled parameter category";
            }

            // BINDINGS HERE
        }

        for (unsigned par_i = 0; par_i < entry_point->getLayout()->getEntryPointByIndex(0)->getParameterCount(); ++par_i)
        {
            auto parameter = entry_point->getLayout()->getEntryPointByIndex(0)->getParameterByIndex(par_i);

            // @TODO Should be marked as PushConstant (slang issue : https://github.com/shader-slang/slang/issues/5676)
            if (parameter->getTypeLayout()->getParameterCategory() == slang::Uniform)
            {
                std::cerr << "push constant size : " << parameter->getTypeLayout()->getSize() << "\n";
            }
        }

        auto binary = kernel_blob->getBufferPointer();

        (void)binary;
    }

    return "Success";
}



int main()
{
	if (SLANG_FAILED(slang::createGlobalSession(global_session.writeRef())))
	{
		std::cerr << "Failed to create global slang compiler session\n";
		return -1;
	}

	slang::SessionDesc sessionDesc;

	// Target
	slang::TargetDesc targetDesc;
	targetDesc.format = SLANG_SPIRV;
	targetDesc.profile = global_session->findProfile("spirv_1_5");
	if (targetDesc.profile == SLANG_PROFILE_UNKNOWN)
	{
		std::cerr << "Failed to find slang profile 'spirv_1_5'\n";
		return -1;
	}
	sessionDesc.targets = &targetDesc;
	sessionDesc.targetCount = 1;

	// Search paths
	const char* searchPaths[] = {"resources/shaders"};
	sessionDesc.searchPaths = searchPaths;
	sessionDesc.searchPathCount = 1;

	if (SLANG_FAILED(global_session->createSession(sessionDesc, session.writeRef())))
	{
		std::cerr << "Failed to create slang compiler session\n";
		return - 1;
	}

	Slang::ComPtr<slang::IBlob> diagnostics;
    std::filesystem::path file_path = std::filesystem::path(__FILE__).parent_path() / "test_shader.slang";
	module = session->loadModule(file_path.string().c_str(), diagnostics.writeRef());
	if (diagnostics)
	{
		std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";
		return -1;
	}

    std::cout << reflection_test() << "\n";
}
