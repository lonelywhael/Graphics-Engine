#ifndef FRAME_HPP
#define FRAME_HPP

#include <iostream>
#include <memory>
#include <vector>

#include "../io/serializer.hpp"

#include "elements.hpp"
#include "frame_buffer.hpp"
#include "render_group.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"

class Frame;
class Pane;

class Frame {
public:
    Frame() : frameBuffer(nullptr) {}
    Frame(std::unique_ptr<FrameBuffer>&& frameBuffer) : frameBuffer(std::move(frameBuffer)) {}
    Frame(Serializer& object);

    void addRenderGroup(std::shared_ptr<RenderGroup> renderGroup) { renderGroups.push_back(renderGroup); }
    int addFrame(std::unique_ptr<Frame>&& frame);
    int addPane(std::shared_ptr<RenderGroup>& renderGroup, std::unique_ptr<Frame>&& pane);
    int addPane(std::shared_ptr<RenderGroup>& renderGroup, std::unique_ptr<Pane>&& pane);

    void render();
    std::shared_ptr<Texture> getFrame() const { return frameBuffer->getBuffer(); }
    bool isDefault() const { return (frameBuffer == nullptr); }

    void print(int tab) const;

    virtual Serializer getJSON();

protected:
    std::unique_ptr<FrameBuffer> frameBuffer;

private:
    std::vector<std::shared_ptr<RenderGroup>> renderGroups;

    std::vector<std::unique_ptr<Frame>> subframes;
    TextureGroup subframes_tg = TextureGroup(FRAME_SLOT);
};

class Pane : public Frame {
public:
    Pane(std::unique_ptr<FrameBuffer>&& frameBuffer);
    Pane(std::unique_ptr<FrameBuffer>&& frameBuffer, float posX, float posY, float dimX, float dimY);
    Pane(Serializer object);

    std::shared_ptr<Model> getPaneModel() const { return paneModel; }

    virtual Serializer getJSON() override;

private:
    float paneDims[4] { -1.0f, -1.0f, 2.0f, 2.0f };
    std::shared_ptr<Model> paneModel;
};


#endif