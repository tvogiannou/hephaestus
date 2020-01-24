import numpy
from PIL import Image
import sys

import hephaestus_bindings


def script_main():

    width = 1024
    height = 1024
    shaderDir = '../data/shaders'   # needs to point to the directory with the hephaestus shaders from the repo
    hephaestus_bindings.init_system(width, height, shaderDir)
    # hephaestus_bindings.set_clear_color(0, 0, 0, 1)

    # points = [0,0,0, 0,1,0, 1,1,0, 1,0,0]
    # indices = [0,1,2, 0, 2, 3]
    points, indices = hephaestus_bindings.load_obj("../data/teddy.obj")

    # create a model from the mean mesh of the LSFM
    model = hephaestus_bindings.create_mesh(points, indices)

    # setup the pipeline so that it can be rendered
    hephaestus_bindings.setup_model(model)
    # hephaestus_bindings.set_light_pos(model, hephaestus_bindings.vec4(0, 0, -10, 0))

    # setup projection w& place the camera
    hephaestus_bindings.set_perspective_projection(model, width / height, 60, 0.1, 100.)
    # hephaestus_bindings.set_orthographics_projection(model, -2, 2, 2, -2, 0.1, 100)
    p = points.reshape(-1, 3)
    center = numpy.mean(p, axis=0)
    camera = 3 * numpy.max(p, axis=0)
    target = hephaestus_bindings.vec4(center[0], center[1], center[2], 1)
    camera = hephaestus_bindings.vec4(camera[0], 3 * center[1], camera[2], 1)
    # target = hephaestus_bindings.vec4(0, 0, 0, 1)
    # camera = hephaestus_bindings.vec4(0, 0, -5, 1)
    hephaestus_bindings.set_camera_lookat(model, camera, target)

    # # render the model using the current settings of the renderer
    data, channels, width, height = hephaestus_bindings.render_mesh(model)
    
    # # convert to RGB array
    dataRGBA = data.reshape((width, height, channels))
    
    # # display frame
    img = Image.fromarray(dataRGBA, mode='RGBA')
    img.show()


    hephaestus_bindings.clear_system()


if __name__ == "__main__":
    script_main()