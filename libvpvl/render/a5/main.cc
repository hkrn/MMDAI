/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_opengl.h>

#include <vpvl/vpvl.h>
#include <vpvl/gl/Renderer.h>

namespace internal
{

static const int kWidth = 800;
static const int kHeight = 600;
static const int kFPS = 60;

static const std::string kSystemDir = "render/res/system";
static const std::string kModelDir = "render/res/default";
static const std::string kStageDir = "render/res/stage";
static const std::string kMotion = "test/res/motion.vmd";
static const std::string kCamera = "test/res/camera.vmd";
static const std::string kModelName = "miku.pmd";
static const std::string kStageName = "stage.x";

static const std::string concatPath(const std::string &dir, const std::string &name) {
    return dir + "/" + name;
}

static void slurpFile(const std::string &path, uint8_t *&data, size_t &size) {
    ALLEGRO_FILE *file = al_fopen(path.c_str(), "rb");
    if (file) {
        size = al_fsize(file);
        data = new uint8_t[size];
        al_fread(file, data, size);
        al_fclose(file);
    }
    else {
        data = 0;
        size = 0;
        fprintf(stderr, "Failed loading file %s\n", path.c_str());
    }
}

class Delegate : public vpvl::gl::Delegate
{
public:
    Delegate(const std::string &system) : m_system(system) {
        al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
    }
    ~Delegate() {
    }

    bool loadTexture(const std::string &path, GLuint &textureID) {
        ALLEGRO_PATH *p = al_create_path(path.c_str());
        ALLEGRO_FILE *file = al_fopen(path.c_str(), "rb");
        if (file) {
            ALLEGRO_BITMAP *bitmap;
            const char *ext = al_get_path_extension(p);
            if (strcmp(ext, ".sph") == 0 || strcmp(ext, ".spa") == 0)
                bitmap = al_load_bitmap_f(file, ".bmp");
            else
                bitmap = al_load_bitmap_f(file, ext);
            if (bitmap) {
                textureID = al_get_opengl_texture(bitmap);
                al_fclose(file);
                al_destroy_path(p);
                return true;
            }
            else {
                fprintf(stderr, "Failed loading image %s\n", path.c_str());
                al_fclose(file);
                al_destroy_path(p);
                return false;
            }
        }
        else {
            fprintf(stderr, "Failed loading image %s\n", path.c_str());
            al_destroy_path(p);
            return false;
        }
    }
    bool loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) {
        std::string path = dir + "/" + name;
        if (!al_filename_exists(path.c_str())) {
            path = m_system + "/" + name;
            if (!al_filename_exists(path.c_str())) {
                fprintf(stderr, "%s is not found, skipped...\n", path.c_str());
                return false;
            }
        }
        return loadTexture(path, textureID);
    }
    const std::string toUnicode(const uint8_t *value) {
        /* do nothing */
        return reinterpret_cast<const char *>(value);
    }

private:
    std::string m_system;
};

}

class UI
{
public:
    UI(int argc, char **argv)
        : m_delegate(internal::kSystemDir),
          m_renderer(&m_delegate, internal::kWidth, internal::kHeight, internal::kFPS),
          m_display(NULL),
          m_queue(NULL),
          m_modelData(0),
          m_argc(argc),
          m_argv(argv)
    {
        if (!al_init()) {
            fprintf(stderr, "failed initializing allegro5\n");
            abort();
        }
    }
    ~UI() {
        al_destroy_event_queue(m_queue);
        al_uninstall_keyboard();
        al_uninstall_system();
        delete[] m_modelData;
    }

    bool initialize() {
        al_install_keyboard();
        al_init_image_addon();
        al_set_new_display_flags(ALLEGRO_OPENGL);
        al_set_new_display_option(ALLEGRO_RED_SIZE, 8, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_GREEN_SIZE, 8, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_BLUE_SIZE, 8, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_ALPHA_SIZE, 8, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 24, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_STENCIL_SIZE, 8, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
        al_set_new_display_option(ALLEGRO_SAMPLES, 4, ALLEGRO_SUGGEST);
        m_display = al_create_display(internal::kWidth, internal::kHeight);
        if (!m_display) {
            fprintf(stderr, "failed initializing display\n");
            return false;
        }
        m_queue = al_create_event_queue();
        m_timer = al_create_timer(1.0 / internal::kFPS);
        al_register_event_source(m_queue, al_get_keyboard_event_source());
        al_register_event_source(m_queue, al_get_display_event_source(m_display));
        al_register_event_source(m_queue, al_get_timer_event_source(m_timer));
        size_t size = 0;
        internal::slurpFile(internal::concatPath(internal::kModelDir, internal::kModelName), m_modelData, size);
        if (!m_model.load(m_modelData, size)) {
            fprintf(stderr, "Failed parsing the model\n");
            return false;
        }
        m_renderer.loadModel(&m_model, internal::kModelDir);
        vpvl::Scene *scene = m_renderer.scene();
        m_renderer.setLighting();
        scene->addModel(&m_model);
        return true;
    }
    int execute() {
        al_start_timer(m_timer);
        while (true) {
            if (!al_is_event_queue_empty(m_queue)) {
                ALLEGRO_EVENT event;
                while (al_get_next_event(m_queue, &event)) {
                    switch (event.type) {
                    case ALLEGRO_EVENT_DISPLAY_CLOSE:
                        return 0;
                    case ALLEGRO_EVENT_KEY_DOWN:
                        if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                            return 0;
                    case ALLEGRO_EVENT_TIMER:
                        update();
                        break;
                    }
                }
            }
            draw();
            al_flip_display();
        }
        return 0;
    }

protected:
    virtual void update() {
        vpvl::Scene *scene = m_renderer.scene();
        scene->updateModelView(0.5);
        scene->updateProjection(0);
        scene->update(0);
    }
    virtual void draw() {
        m_renderer.initializeSurface();
        m_renderer.drawSurface();
    }

private:
    internal::Delegate m_delegate;
    vpvl::gl::Renderer m_renderer;
    vpvl::PMDModel m_model;
    ALLEGRO_DISPLAY *m_display;
    ALLEGRO_EVENT_QUEUE *m_queue;
    ALLEGRO_TIMER *m_timer;
    uint8_t *m_modelData;
    int m_argc;
    char **m_argv;
};

int main(int argc, char *argv[])
{
    UI ui(argc, argv);
    if (!ui.initialize())
        return -1;
    return ui.execute();
}
