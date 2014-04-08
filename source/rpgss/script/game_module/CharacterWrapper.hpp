#ifndef RPGSS_SCRIPT_GAME_MODULE_CHARACTERWRAPPER_HPP_INCLUDED
#define RPGSS_SCRIPT_GAME_MODULE_CHARACTERWRAPPER_HPP_INCLUDED

#include "../lua_include.hpp"


namespace RPG {

    // forward declaration
    class Character;

}

namespace rpgss {
    namespace script {
        namespace game_module {

            class CharacterWrapper {
            public:
                static void Push(lua_State* L, RPG::Character* character);
                static bool Is(lua_State* L, int index);
                static RPG::Character* Get(lua_State* L, int index);
                static RPG::Character* GetOpt(lua_State* L, int index);

                explicit CharacterWrapper(RPG::Character* ptr);

                std::string get_name() const;

                int get_id() const;

                std::string get_charsetFilename() const;

                int get_charsetId() const;

                bool get_enabled() const;
                void set_enabled(bool enabled);

                int get_screenX() const;

                int get_screenY() const;

                int  get_x() const;
                void set_x(int x);

                int  get_y() const;
                void set_y(int y);

                int  get_offsetX() const;
                void set_offsetX(int offsetX);

                int  get_offsetY() const;
                void set_offsetY(int offsetY);

                std::string get_direction() const;
                void        set_direction(const std::string& direction_str);

                std::string get_facing() const;
                void        set_facing(const std::string& facing_str);

                int  get_step() const;
                void set_step(int step);

                int  get_transparency() const;
                void set_transparency(int transparency);

                int  get_frequency() const;
                void set_frequency(int frequency);

                std::string get_layer() const;
                void        set_layer(const std::string& layer_str);

                bool get_forbidEventOverlap() const;
                void set_forbidEventOverlap(bool forbidEventOverlap);

                std::string get_animationType() const;
                void        set_animationType(const std::string& animationType_str);

                bool get_fixedDirection() const;
                void set_fixedDirection(bool fixedDirection);

                int  get_speed() const;
                void set_speed(int speed);

                bool get_moving() const;
                void set_moving(bool moving);

                bool get_phasing() const;
                void set_phasing(bool phasing);

                int getScreenPosition(lua_State* L);
                int getPosition(lua_State* L);
                int setPosition(lua_State* L);
                int getPositionOffset(lua_State* L);
                int setPositionOffset(lua_State* L);
                int setFixedStep(lua_State* L);
                int setNormalStep(lua_State* L);
                int isMovePossible(lua_State* L);
                int stop(lua_State* L);
                int doStep(lua_State* L);

            private:
                RPG::Character* This;
            };

        } // game_module
    } // namespace script
} // namespace rpgss


#endif // RPGSS_SCRIPT_GAME_MODULE_CHARACTERWRAPPER_HPP_INCLUDED
