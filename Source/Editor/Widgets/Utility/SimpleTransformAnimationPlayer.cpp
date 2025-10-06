//
// Created by MarvelLi on 2025/9/29.
//

#include "SimpleTransformAnimationPlayer.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include "Components/StaticMeshComponent.h"
#include "Math/Math.h"
#include "Misc/Path.h"

void SimpleTransformAnimationPlayer::Draw()
{
    ImGui::Begin("SimpleTransformAnimationPlayer");

    // Animation Controls
    ImGui::Text("Animation Player");
    ImGui::Separator();

    // Duration control
    float durationFloat = static_cast<float>(Duration);
    if (ImGui::InputFloat("Duration (s)", &durationFloat, 0.1f, 1.0f, "%.2f")) {
        Duration = static_cast<double>(durationFloat);
    }

    // Progress slider
    float percentFloat = static_cast<float>(Percent);
    if (ImGui::SliderFloat("Progress", &percentFloat, 0.0f, 1.0f, "%.3f")) {
        Percent = static_cast<double>(percentFloat);
        ApplyCurrentFrame();
    }

    // Playback controls
    ImGui::Spacing();
    if (ImGui::Button("Play")) {
        lastTime   = ImGui::GetTime();
        bIsPlaying = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        bIsPlaying = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        bIsPlaying = false;
        Percent    = 0.0;
        ApplyCurrentFrame();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        Percent    = 0.0;
        bIsPlaying = false;
        ApplyCurrentFrame();
    }
    if (ImGui::Button("Export")) {
        auto FilePath = SaveFileDialog("Export GLB", Path::ProjectContentDir()).result();
        if (Path(FilePath).extension().string() == ".glb") {
            ExportToGltf(FilePath, true);
            ImGui::NotifySuccess("Exported successfully to " + FilePath);
        }
        else if (Path(FilePath).extension().string() == ".gltf") {
            ExportToGltf(FilePath, false);
            ImGui::NotifySuccess("Exported successfully to " + FilePath);

        }
        else {
            ImGui::NotifyError("Please select a valid .glb or .gltf file path.");
        }
    }

    // Time display
    double currentTime = Percent * Duration;
    ImGui::Text("Time: %.2f / %.2f seconds", currentTime, Duration);

    // Track information
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Tracks (%zu)", Tracks.size());

    // Display tracks
    for (size_t i = 0; i < Tracks.size(); ++i) {
        const Track& track = Tracks[i];

        ImGui::PushID(static_cast<int>(i));

        // Track header
        bool trackOpen = ImGui::TreeNode(("Track " + std::to_string(i)).c_str());

        // Track controls on same line
        ImGui::SameLine();
        if (ImGui::SmallButton("Remove")) {
            Tracks.erase(Tracks.begin() + i);
            ImGui::PopID();
            break; // Exit loop since we modified the container
        }

        if (trackOpen) {
            // Actor information
            if (track.Actor) {
                ImGui::Text("Actor: Valid (%s)", track.Actor->GetName().c_str());
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Actor: Invalid/Null");
            }

            // Transform count
            ImGui::Text("Keyframes: %zu", track.Transforms.size());

            // Show current transform if valid
            if (!track.Transforms.empty() && track.Actor) {
                int currentKeyframe = static_cast<int>(Percent * (track.Transforms.size() - 1));
                currentKeyframe     = std::min(currentKeyframe, static_cast<int>(track.Transforms.size() - 1));

                const FTransform& currentTransform = track.Transforms[currentKeyframe];
                ImGui::Text("Current Keyframe: %d", currentKeyframe);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    // Update animation if playing
    if (bIsPlaying) {
        // Simple time update (you'd typically use a proper timer)
        double currentFrameTime = ImGui::GetTime();
        double deltaTime        = currentFrameTime - lastTime;
        lastTime                = currentFrameTime;

        Percent += deltaTime / Duration;

        if (Percent >= 1.0) {
            if (bLoop) {
                Percent = 0.0;
            }
            else {
                Percent    = 1.0;
                bIsPlaying = false;
            }
        }

        ApplyCurrentFrame();
    }

    // Animation settings
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Checkbox("Loop", &bLoop);

    ImGui::End();
}
void SimpleTransformAnimationPlayer::ExportToGltf(std::string& Path, bool bBinary)
{
    tinygltf::Model model;
    tinygltf::Scene scene;

    model.asset.version = "2.0";
    model.asset.generator = "SimpleTransformAnimationPlayer";

    // Create buffers for all data
    tinygltf::Buffer buffer;
    std::vector<unsigned char> bufferData;

    // Animation data
    tinygltf::Animation animation;
    animation.name = "Animation_001";

    // Process each track
    for (size_t trackIndex = 0; trackIndex < Tracks.size(); ++trackIndex)
    {
        const Track& track = Tracks[trackIndex];

        if (!track.Actor || track.Transforms.empty())
            continue;

        // Create node for this actor
        tinygltf::Node node;
        node.name = track.Actor->GetName() + "_" + std::to_string(trackIndex);

        // Export mesh if available
        if (track.Actor)
        {
            auto Mesh = track.Actor->GetComponent<StaticMeshComponent>()->GetStaticMesh();
            const auto& verM = Mesh->verM;
            const auto& triM = Mesh->triM;

            TArray<Eigen::Vector3f> vertices;
            for (int i = 0; i < verM.rows(); i++)
            {
                auto v = verM.row(i);
                vertices.emplace_back(v.x(), v.y(), v.z());
            }
            TArray<unsigned int> indices;
            for (int i = 0; i < triM.rows(); i++)
            {
                auto t = triM.row(i);
                indices.emplace_back(static_cast<unsigned int>(t.x()));
                indices.emplace_back(static_cast<unsigned int>(t.y()));
                indices.emplace_back(static_cast<unsigned int>(t.z()));
            }

            // Create mesh
            tinygltf::Mesh mesh;
            mesh.name = node.name + "_Mesh";
            tinygltf::Primitive primitive;

            // --- POSITION ATTRIBUTE ---
            size_t positionBufferStart = bufferData.size();
            for (const auto& vertex : vertices)
            {
                float v[3] = {vertex.x(), vertex.y(), vertex.z()};
                bufferData.insert(bufferData.end(),
                                reinterpret_cast<unsigned char*>(v),
                                reinterpret_cast<unsigned char*>(v) + sizeof(v));
            }

            tinygltf::BufferView positionBufferView;
            positionBufferView.buffer = 0;
            positionBufferView.byteOffset = positionBufferStart;
            positionBufferView.byteLength = vertices.size() * sizeof(float) * 3;
            positionBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
            int positionBufferViewIndex = model.bufferViews.size();
            model.bufferViews.push_back(positionBufferView);

            tinygltf::Accessor positionAccessor;
            positionAccessor.bufferView = positionBufferViewIndex;
            positionAccessor.byteOffset = 0;
            positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            positionAccessor.count = vertices.size();
            positionAccessor.type = TINYGLTF_TYPE_VEC3;

            // Calculate min/max for positions
            if (!vertices.empty())
            {
                positionAccessor.minValues = {vertices[0].x(), vertices[0].y(), vertices[0].z()};
                positionAccessor.maxValues = {vertices[0].x(), vertices[0].y(), vertices[0].z()};
                for (const auto& v : vertices)
                {
                    positionAccessor.minValues[0] = std::min(positionAccessor.minValues[0], static_cast<double>(v.x()));
                    positionAccessor.minValues[1] = std::min(positionAccessor.minValues[1], static_cast<double>(v.y()));
                    positionAccessor.minValues[2] = std::min(positionAccessor.minValues[2], static_cast<double>(v.z()));
                    positionAccessor.maxValues[0] = std::max(positionAccessor.maxValues[0], static_cast<double>(v.x()));
                    positionAccessor.maxValues[1] = std::max(positionAccessor.maxValues[1], static_cast<double>(v.y()));
                    positionAccessor.maxValues[2] = std::max(positionAccessor.maxValues[2], static_cast<double>(v.z()));
                }
            }

            int positionAccessorIndex = model.accessors.size();
            model.accessors.push_back(positionAccessor);
            primitive.attributes["POSITION"] = positionAccessorIndex;

            // --- INDICES ---
            // Determine the appropriate index type based on vertex count
            size_t indexBufferStart = bufferData.size();
            int indexComponentType;

            if (vertices.size() <= 255)
            {
                // Use unsigned byte for small meshes
                indexComponentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
                for (const auto& index : indices)
                {
                    unsigned char idx = static_cast<unsigned char>(index);
                    bufferData.push_back(idx);
                }
            }
            else if (vertices.size() <= 65535)
            {
                // Use unsigned short for medium meshes
                indexComponentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                for (const auto& index : indices)
                {
                    unsigned short idx = static_cast<unsigned short>(index);
                    bufferData.insert(bufferData.end(),
                                    reinterpret_cast<unsigned char*>(&idx),
                                    reinterpret_cast<unsigned char*>(&idx) + sizeof(idx));
                }
            }
            else
            {
                // Use unsigned int for large meshes
                indexComponentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
                for (const auto& index : indices)
                {
                    unsigned int idx = index;
                    bufferData.insert(bufferData.end(),
                                    reinterpret_cast<unsigned char*>(&idx),
                                    reinterpret_cast<unsigned char*>(&idx) + sizeof(idx));
                }
            }

            tinygltf::BufferView indexBufferView;
            indexBufferView.buffer = 0;
            indexBufferView.byteOffset = indexBufferStart;
            indexBufferView.byteLength = bufferData.size() - indexBufferStart;
            indexBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
            int indexBufferViewIndex = model.bufferViews.size();
            model.bufferViews.push_back(indexBufferView);

            tinygltf::Accessor indexAccessor;
            indexAccessor.bufferView = indexBufferViewIndex;
            indexAccessor.byteOffset = 0;
            indexAccessor.componentType = indexComponentType;
            indexAccessor.count = indices.size();
            indexAccessor.type = TINYGLTF_TYPE_SCALAR;

            int indexAccessorIndex = model.accessors.size();
            model.accessors.push_back(indexAccessor);
            primitive.indices = indexAccessorIndex;

            primitive.mode = TINYGLTF_MODE_TRIANGLES;
            mesh.primitives.push_back(primitive);

            int meshIndex = model.meshes.size();
            model.meshes.push_back(mesh);
            node.mesh = meshIndex;
        }

        // --- ANIMATION DATA ---
        // Time data for keyframes
        std::vector<float> times;
        for (size_t i = 0; i < track.Transforms.size(); ++i)
        {
            float t = static_cast<float>(i) / (track.Transforms.size() - 1) * Duration;
            times.push_back(t);
        }

        // Create time accessor
        size_t timeBufferStart = bufferData.size();
        bufferData.insert(bufferData.end(),
                         reinterpret_cast<unsigned char*>(times.data()),
                         reinterpret_cast<unsigned char*>(times.data() + times.size()));

        tinygltf::BufferView timeBufferView;
        timeBufferView.buffer = 0;
        timeBufferView.byteOffset = timeBufferStart;
        timeBufferView.byteLength = times.size() * sizeof(float);
        int timeBufferViewIndex = model.bufferViews.size();
        model.bufferViews.push_back(timeBufferView);

        tinygltf::Accessor timeAccessor;
        timeAccessor.bufferView = timeBufferViewIndex;
        timeAccessor.byteOffset = 0;
        timeAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        timeAccessor.count = times.size();
        timeAccessor.type = TINYGLTF_TYPE_SCALAR;
        timeAccessor.minValues = {static_cast<double>(times.front())};
        timeAccessor.maxValues = {static_cast<double>(times.back())};

        int timeAccessorIndex = model.accessors.size();
        model.accessors.push_back(timeAccessor);

        // Translation animation
        {
            std::vector<float> translations;
            for (const auto& transform : track.Transforms)
            {
                auto t = transform.GetTranslation();
                translations.push_back(t.x());
                translations.push_back(t.y());
                translations.push_back(t.z());
            }

            size_t translationBufferStart = bufferData.size();
            bufferData.insert(bufferData.end(),
                             reinterpret_cast<unsigned char*>(translations.data()),
                             reinterpret_cast<unsigned char*>(translations.data() + translations.size()));

            tinygltf::BufferView translationBufferView;
            translationBufferView.buffer = 0;
            translationBufferView.byteOffset = translationBufferStart;
            translationBufferView.byteLength = translations.size() * sizeof(float);
            int translationBufferViewIndex = model.bufferViews.size();
            model.bufferViews.push_back(translationBufferView);

            tinygltf::Accessor translationAccessor;
            translationAccessor.bufferView = translationBufferViewIndex;
            translationAccessor.byteOffset = 0;
            translationAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            translationAccessor.count = track.Transforms.size();
            translationAccessor.type = TINYGLTF_TYPE_VEC3;

            int translationAccessorIndex = model.accessors.size();
            model.accessors.push_back(translationAccessor);

            tinygltf::AnimationChannel channel;
            channel.sampler = animation.samplers.size();
            channel.target_node = model.nodes.size();
            channel.target_path = "translation";
            animation.channels.push_back(channel);

            tinygltf::AnimationSampler sampler;
            sampler.input = timeAccessorIndex;
            sampler.output = translationAccessorIndex;
            sampler.interpolation = "LINEAR";
            animation.samplers.push_back(sampler);
        }

        // Rotation animation
        {
            std::vector<float> rotations;
            for (const auto& transform : track.Transforms)
            {
                auto r = transform.GetRotation();
                rotations.push_back(r.x());
                rotations.push_back(r.y());
                rotations.push_back(r.z());
                rotations.push_back(r.w());
            }

            size_t rotationBufferStart = bufferData.size();
            bufferData.insert(bufferData.end(),
                             reinterpret_cast<unsigned char*>(rotations.data()),
                             reinterpret_cast<unsigned char*>(rotations.data() + rotations.size()));

            tinygltf::BufferView rotationBufferView;
            rotationBufferView.buffer = 0;
            rotationBufferView.byteOffset = rotationBufferStart;
            rotationBufferView.byteLength = rotations.size() * sizeof(float);
            int rotationBufferViewIndex = model.bufferViews.size();
            model.bufferViews.push_back(rotationBufferView);

            tinygltf::Accessor rotationAccessor;
            rotationAccessor.bufferView = rotationBufferViewIndex;
            rotationAccessor.byteOffset = 0;
            rotationAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            rotationAccessor.count = track.Transforms.size();
            rotationAccessor.type = TINYGLTF_TYPE_VEC4;

            int rotationAccessorIndex = model.accessors.size();
            model.accessors.push_back(rotationAccessor);

            tinygltf::AnimationChannel channel;
            channel.sampler = animation.samplers.size();
            channel.target_node = model.nodes.size();
            channel.target_path = "rotation";
            animation.channels.push_back(channel);

            tinygltf::AnimationSampler sampler;
            sampler.input = timeAccessorIndex;
            sampler.output = rotationAccessorIndex;
            sampler.interpolation = "LINEAR";
            animation.samplers.push_back(sampler);
        }

        // Scale animation
        {
            std::vector<float> scales;
            for (const auto& transform : track.Transforms)
            {
                auto s = transform.GetScale();
                scales.push_back(s.x());
                scales.push_back(s.y());
                scales.push_back(s.z());
            }

            size_t scaleBufferStart = bufferData.size();
            bufferData.insert(bufferData.end(),
                             reinterpret_cast<unsigned char*>(scales.data()),
                             reinterpret_cast<unsigned char*>(scales.data() + scales.size()));

            tinygltf::BufferView scaleBufferView;
            scaleBufferView.buffer = 0;
            scaleBufferView.byteOffset = scaleBufferStart;
            scaleBufferView.byteLength = scales.size() * sizeof(float);
            int scaleBufferViewIndex = model.bufferViews.size();
            model.bufferViews.push_back(scaleBufferView);

            tinygltf::Accessor scaleAccessor;
            scaleAccessor.bufferView = scaleBufferViewIndex;
            scaleAccessor.byteOffset = 0;
            scaleAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            scaleAccessor.count = track.Transforms.size();
            scaleAccessor.type = TINYGLTF_TYPE_VEC3;

            int scaleAccessorIndex = model.accessors.size();
            model.accessors.push_back(scaleAccessor);

            tinygltf::AnimationChannel channel;
            channel.sampler = animation.samplers.size();
            channel.target_node = model.nodes.size();
            channel.target_path = "scale";
            animation.channels.push_back(channel);

            tinygltf::AnimationSampler sampler;
            sampler.input = timeAccessorIndex;
            sampler.output = scaleAccessorIndex;
            sampler.interpolation = "LINEAR";
            animation.samplers.push_back(sampler);
        }

        // Add node to scene
        int nodeIndex = model.nodes.size();
        model.nodes.push_back(node);
        scene.nodes.push_back(nodeIndex);
    }

    // Add animation to model
    if (!animation.channels.empty())
    {
        model.animations.push_back(animation);
    }

    // Finalize buffer
    buffer.data = bufferData;
    if (bBinary)
    {
        buffer.uri = ""; // Empty URI for GLB embedded buffer
    }
    else
    {
        buffer.uri = Path + ".bin";
    }
    model.buffers.push_back(buffer);

    // Add scene to model
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    // Validate the model before writing
    tinygltf::TinyGLTF writer;
    std::string err;
    std::string warn;

    if (!writer.WriteGltfSceneToFile(&model, Path, false, true, true, bBinary))
    {
    }
    else
    {
        LOG_INFO("Successfully exported GLTF/GLB to: {}", Path);

        // Additional validation using TinyGLTF's validator
        tinygltf::Model validationModel;
        std::string validationErr;
        std::string validationWarn;

        if (bBinary)
        {
            if (!writer.LoadBinaryFromFile(&validationModel, &validationErr, &validationWarn, Path))
            {
                LOG_ERROR("Validation failed for exported GLB: Error:{} Warn:{}", validationErr, validationWarn);
            }
            else
            {
                LOG_INFO("GLB validation passed");
            }
        }
        else
        {
            if (!writer.LoadASCIIFromFile(&validationModel, &validationErr, &validationWarn, Path))
            {
                LOG_ERROR("Validation failed for exported GLTF: {}", validationErr);
            }
            else
            {
                LOG_INFO("GLTF validation passed");
            }
        }
    }
}
void SimpleTransformAnimationPlayer::ApplyCurrentFrame()
{
    for (const Track& track : Tracks)
    {
        if (!track.Actor || track.Transforms.empty())
            continue;

        // Calculate interpolated transform
        FTransform interpolatedTransform = GetInterpolatedTransform(track.Transforms, Percent);

        // Apply transform to actor (assuming Actor has a SetTransform method)
        track.Actor->SetTransform(interpolatedTransform);
    }
}

FTransform SimpleTransformAnimationPlayer::GetInterpolatedTransform(const TArray<FTransform>& Transforms,
                                                                    double InPercent)
{
    if (Transforms.empty())
        return FTransform::Identity();

    if (Transforms.size() == 1)
        return Transforms[0];

    // Calculate indices for interpolation
    double scaledPercent = InPercent * (Transforms.size() - 1);
    int lowerIndex = static_cast<int>(floor(scaledPercent));
    int upperIndex = lowerIndex + 1;

    // Clamp indices
    lowerIndex = std::max(0, std::min(lowerIndex, static_cast<int>(Transforms.size() - 1)));
    upperIndex = std::max(0, std::min(upperIndex, static_cast<int>(Transforms.size() - 1)));

    // If indices are the same, no interpolation needed
    if (lowerIndex == upperIndex)
        return Transforms[lowerIndex];

    // Calculate interpolation factor
    double t = scaledPercent - lowerIndex;

    // Interpolate between transforms
    const FTransform& transform1 = Transforms[lowerIndex];
    const FTransform& transform2 = Transforms[upperIndex];

    FTransform result;
    result.SetTranslation(Math::Lerp(transform1.GetTranslation(), transform2.GetTranslation(), t));
    result.SetRotation(transform1.GetRotation().slerp(t, transform2.GetRotation()));
    result.SetScale(Math::Lerp(transform1.GetScale(), transform2.GetScale(), t));

    return result;
}
