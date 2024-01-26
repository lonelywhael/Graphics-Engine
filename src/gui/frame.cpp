#include "gui/frame.hpp"

Frame::Frame(Format& object) {
    /*frameBuffer = (object["frame_buffer"] != nullptr) ? new FrameBuffer(static_cast<Format&>(object["frame_buffer"])) : nullptr;
    for (int i = 0; i < object["render_groups"].size(); i++)
        renderGroups.push_back(new RenderGroup(static_cast<Format&>(object["render_groups"][i])));
    for (int i = 0; i < object["subframes"].size(); i++) {
        Frame* subframe = new Frame(static_cast<Format&>(object["subframes"][i]));
        subframes.push_back(subframe);
        subframes_tg.addTexture(subframe->getFrame());
    }*/
}

int Frame::addFrame(std::unique_ptr<Frame>&& frame) {
    if (frame.get() != this) subframes.push_back(std::move(frame));
    int slot = subframes_tg.addTexture(subframes.back()->getFrame());

    return slot;
}
int Frame::addPane(std::shared_ptr<RenderGroup>& renderGroup, std::unique_ptr<Frame>&& pane) {
    std::shared_ptr<Model> pane_model = dynamic_cast<Pane&>(*pane).getPaneModel();
    renderGroup->addModel(pane_model);
    addRenderGroup(renderGroup);

    return addFrame(std::move(pane));
}
int Frame::addPane(std::shared_ptr<RenderGroup>& renderGroup, std::unique_ptr<Pane>&& pane) {
    std::shared_ptr<Model> pane_model = pane->getPaneModel();
    renderGroup->addModel(pane_model);
    addRenderGroup(renderGroup);

    return addFrame(std::move(pane));
}

void Frame::render() {
    for (int i = 0; i < subframes.size(); i++) subframes[i]->render();

    // enable all the correct rendering settings based on the frame being used
    if (frameBuffer.get() != nullptr) {   // if the frame is assigned, use the assigned frame
        frameBuffer->clear();
        // bind the frame so that all rendering occurs on the correct frame
        frameBuffer->bind();
        // enable the depth buffer if one is being used
        if (frameBuffer->isDepthEnabled()) r_EnableDepthBuffer();
        else r_DisableDepthBuffer();
        // in the case of anti aliasing, need to tell the renderer to optimize for sampling across multiple adjacent pixels
        if (frameBuffer->isAntiAliasingEnabled()) r_EnableMultisample();
        else r_DisableMultisample();
    } else {    // otherwise, use the default frame
        // as above, just checking the status of the default frame instead of a specific frame
        CLEAR_DEFAULT_FRAME();
        BIND_DEFAULT_FRAME();
        if (DEPTH_TESTING_ENABLED) r_EnableDepthBuffer();
        else r_DisableDepthBuffer();
        if (ANTI_ALIASING_ENABLED) r_EnableMultisample();
        else r_DisableMultisample();
    }
    subframes_tg.bind();

    // render all groups
    for (int i = 0; i < renderGroups.size(); i++) renderGroups[i]->render();

    // in the case of antialiasing, need to apply anti-aliasing as a post-processing step
    if (frameBuffer != nullptr && frameBuffer->isAntiAliasingEnabled()) frameBuffer->applyAntiAliasing();
}

void Frame::print(int tab) const {
    /*std::string tabs(tab, '\t');
    for (int i = 0; i < renderGroups.size(); i++) {
        std::cout << tabs << "Render Group " << i << ": " << renderGroups[i] << std::endl;
        renderGroups[i]->print(tab + 2);
    }
    for (int i = 0; i < subframes.size(); i++) {
        std::cout << tabs << "Sub Frame " << i << ": " << subframes[i] << std::endl;
        subframes[i]->print(tab + 2);
    }*/
}

Format Frame::getJSON() {
    Format object;
    (frameBuffer == nullptr) ? object["frame_buffer"] = nullptr : object["frame_buffer"] = std::move(frameBuffer->getJSON());
    for (int i = 0; i < renderGroups.size(); i++) object["render_groups"][i] = std::move(renderGroups[i]->getJSON());
    for (int i = 0; i < subframes.size(); i++) object["subframes"][i] = std::move(subframes[i]->getJSON());

    return object;
}

//---------------------------------------------------------------------------------------------------------------------------------------

Pane::Pane(std::unique_ptr<FrameBuffer>&& frameBuffer) 
        : Frame(std::move(frameBuffer)) {
    std::shared_ptr<VertexArray> paneGeometry = std::make_shared<VertexArray>(); 
    paneGeometry->makePane();
    paneModel = std::make_shared<Model>(paneGeometry, this->frameBuffer->getBuffer());
}
Pane::Pane(std::unique_ptr<FrameBuffer>&& frameBuffer, float posX, float posY, float dimX, float dimY)
        : Frame(std::move(frameBuffer)) {
    paneDims[0] = posX; paneDims[1] = posY; paneDims[2] = dimX; paneDims[3] = dimY;
    std::shared_ptr<VertexArray> paneGeometry = std::make_shared<VertexArray>(); 
    paneGeometry->makePane(posX, posY, dimX, dimY);
    paneModel = std::make_shared<Model>(paneGeometry, this->frameBuffer->getBuffer());
}
Pane::Pane(Format object) : Frame(object) {
    for (int i = 0; i < 4; i++) paneDims[i] = static_cast<float>(object["pane_dims"][i]);
    std::shared_ptr<VertexArray> paneGeometry = std::make_shared<VertexArray>();
    paneGeometry->makePane(paneDims[0], paneDims[1], paneDims[2], paneDims[3]);
    paneModel = std::make_shared<Model>(paneGeometry, frameBuffer->getBuffer());
}

Format Pane::getJSON() {
    Format object = Frame::getJSON();
    object["pane_geometry"] = { paneDims[0], paneDims[1], paneDims[2], paneDims[3] };

    return object;
}