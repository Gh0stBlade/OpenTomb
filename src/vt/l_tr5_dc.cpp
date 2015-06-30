
/*
 * Copyright 2002 - Florian Schulze <crow@icculus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This file is part of vt.
 *
 */

#include <SDL2/SDL.h>
#include "l_main.h"
#include "../system.h"
#include "../audio.h"

#define RCSID "$Id: l_tr5.cpp,v 1.14 2002/09/20 15:59:02 crow Exp $"

#define DREAMCAST_BETA 0 //TOM.TRC, GIBBY.TRC, DEL.TRC ONLY sound map size is different

#if DREAMCAST_BETA
#define TR_AUDIO_MAP_SIZE_TR5 370
#endif

void TR_Level::read_tr5_dc_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & vert)
{
    read_tr4_vertex_float(src, vert.vertex);
    read_tr4_vertex_float(src, vert.normal);
    vert.colour.b = read_bitu8(src) / 255.0f;
    vert.colour.g = read_bitu8(src) / 255.0f;
    vert.colour.r = read_bitu8(src) / 255.0f;
    vert.colour.a = read_bitu8(src) / 255.0f;
    if (read_bitu32(src) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room_vertex: seperator23 has wrong value");
}

void TR_Level::read_tr5_dc_room(SDL_RWops * const src, tr5_room_t & room)
{
    uint32_t room_data_size;
    //uint32_t portal_offset;
    uint32_t sector_data_offset;
    uint32_t static_meshes_offset;
    uint32_t layer_offset;
    uint32_t vertices_offset;
    uint32_t poly_offset;
    uint32_t poly_offset2;
    uint32_t vertices_size;
    //uint32_t light_size;                            ///@FIXME: set, but not used

    SDL_RWops *newsrc = NULL;
    uint32_t temp;
    uint32_t i;
    uint8_t *buffer;

    if (read_bitu32(src) != 0x414C4558)
        Sys_extError("read_tr5_dc_room: 'XELA' not found");

    room_data_size = read_bitu32(src);
    buffer = new uint8_t[room_data_size];

    if (SDL_RWread(src, buffer, 1, room_data_size) < room_data_size)
        Sys_extError("read_tr5_dc_room: room_data");

    if ((newsrc = SDL_RWFromMem(buffer, room_data_size)) == NULL)
        Sys_extError("read_tr5_dc_room: SDL_RWFromMem");

    room.intensity1 = 32767;
    room.intensity2 = 32767;
    room.light_mode = 0;
    room.alternate_room = 0;

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator1 has wrong value");

     /*portal_offset = */read_bit32(newsrc);             // StartPortalOffset?   // endSDOffset
    sector_data_offset = read_bitu32(newsrc);    // StartSDOffset
    temp = read_bitu32(newsrc);
    if ((temp != 0) && (temp != 0xCDCDCDCD))
        Sys_extWarn("read_tr5_dc_room: seperator2 has wrong value");

    static_meshes_offset = read_bitu32(newsrc);     // endPortalOffset
    // static_meshes_offset or room_layer_offset
    // read and change coordinate system
    room.offset.x = (float)read_bit32(newsrc);
    room.offset.y = read_bitu32(newsrc);
    room.offset.z = (float)-read_bit32(newsrc);
    room.y_bottom = (float)-read_bit32(newsrc);
    room.y_top = (float)-read_bit32(newsrc);

    room.num_zsectors = read_bitu16(newsrc);
    room.num_xsectors = read_bitu16(newsrc);

    room.light_colour.b = read_bitu8(newsrc) / 255.0f;
    room.light_colour.g = read_bitu8(newsrc) / 255.0f;
    room.light_colour.r = read_bitu8(newsrc) / 255.0f;
    room.light_colour.a = read_bitu8(newsrc) / 255.0f;

    room.num_lights = read_bitu16(newsrc);
    if (room.num_lights > 512)
        Sys_extWarn("read_tr5_dc_room: num_lights > 512");

    room.num_static_meshes = read_bitu16(newsrc);
    if (room.num_static_meshes > 512)
        Sys_extWarn("read_tr5_dc_room: num_static_meshes > 512");

    room.reverb_info = read_bitu8(newsrc);
    room.alternate_group = read_bitu8(newsrc);
    room.water_scheme = read_bitu16(newsrc);

    if (read_bitu32(newsrc) != 0x00007FFF)
        Sys_extWarn("read_tr5_dc_room: filler1 has wrong value");

    if (read_bitu32(newsrc) != 0x00007FFF)
        Sys_extWarn("read_tr5_dc_room: filler2 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator4 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator5 has wrong value");

    if (read_bitu32(newsrc) != 0xFFFFFFFF)
        Sys_extWarn("read_tr5_dc_room: seperator6 has wrong value");

    room.alternate_room = read_bit16(newsrc);

    room.flags = read_bitu16(newsrc);

    room.unknown_r1 = read_bitu32(newsrc);
    room.unknown_r2 = read_bitu32(newsrc);
    room.unknown_r3 = read_bitu32(newsrc);

    temp = read_bitu32(newsrc);
    if ((temp != 0) && (temp != 0xCDCDCDCD))
        Sys_extWarn("read_tr5_dc_room: seperator7 has wrong value");

    room.unknown_r4a = read_bitu16(newsrc);
    room.unknown_r4b = read_bitu16(newsrc);

    room.room_x = read_float(newsrc);
    room.unknown_r5 = read_bitu32(newsrc);
    room.room_z = -read_float(newsrc);

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator8 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator9 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator10 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator11 has wrong value");

    temp = read_bitu32(newsrc);
    if ((temp != 0) && (temp != 0xCDCDCDCD))
        Sys_extWarn("read_tr5_dc_room: seperator12 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator13 has wrong value");

    room.num_triangles = read_bitu32(newsrc);
    if (room.num_triangles == 0xCDCDCDCD)
            room.num_triangles = 0;
    if (room.num_triangles > 512)
        Sys_extWarn("read_tr5_dc_room: num_triangles > 512");

    room.num_rectangles = read_bitu32(newsrc);
    if (room.num_rectangles == 0xCDCDCDCD)
            room.num_rectangles = 0;
    if (room.num_rectangles > 1024)
        Sys_extWarn("read_tr5_dc_room: num_rectangles > 1024");

    if (read_bitu32(newsrc) != 0)
        Sys_extWarn("read_tr5_dc_room: seperator14 has wrong value");

     /*light_size = */read_bitu32(newsrc);
    if (read_bitu32(newsrc) != room.num_lights)
        Sys_extError("read_tr5_room: room.num_lights2 != room.num_lights");

    room.unknown_r6 = read_bitu32(newsrc);
    room.room_y_top = -read_float(newsrc);
    room.room_y_bottom = -read_float(newsrc);

    room.num_layers = read_bitu32(newsrc);

    /*
       if (room.num_layers != 0) {
       if (room.x != room.room_x)
       throw TR_ReadError("read_tr5_dc_room: x != room_x");
       if (room.z != room.room_z)
       throw TR_ReadError("read_tr5_dc_room: z != room_z");
       if (room.y_top != room.room_y_top)
       throw TR_ReadError("read_tr5_dc_room: y_top != room_y_top");
       if (room.y_bottom != room.room_y_bottom)
       throw TR_ReadError("read_tr5_dc_room: y_bottom != room_y_bottom");
       }
     */

    layer_offset = read_bitu32(newsrc);
    vertices_offset = read_bitu32(newsrc);
    poly_offset = read_bitu32(newsrc);
    poly_offset2 = read_bitu32(newsrc);
    if (poly_offset != poly_offset2)
        Sys_extError("read_tr5_dc_room: poly_offset != poly_offset2");

    vertices_size = read_bitu32(newsrc);
    if ((vertices_size % 32) != 0)
        Sys_extError("read_tr5_dc_room: vertices_size has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator15 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator16 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator17 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator18 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator19 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator20 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator21 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_dc_room: seperator22 has wrong value");

    room.lights = (tr5_room_light_t*)malloc(room.num_lights * sizeof(tr5_room_light_t));
    for (i = 0; i < room.num_lights; i++)
        read_tr5_room_light(newsrc, room.lights[i]);

    SDL_RWseek(newsrc, 224 + sector_data_offset, SEEK_SET);

    room.sector_list = (tr_room_sector_t*)malloc(room.num_zsectors * room.num_xsectors * sizeof(tr_room_sector_t));
    for (i = 0; i < (uint32_t)(room.num_zsectors * room.num_xsectors); i++)
        read_tr_room_sector(newsrc, room.sector_list[i]);

    /*
       if (room.portal_offset != 0xFFFFFFFF) {
       if (room.portal_offset != (room.sector_data_offset + (room.num_zsectors * room.num_xsectors * 8)))
       throw TR_ReadError("read_tr5_dc_room: portal_offset has wrong value");

       SDL_RWseek(newsrc, 208 + room.portal_offset, SEEK_SET);
       }
     */

    room.num_portals = read_bit16(newsrc);
    room.portals = (tr_room_portal_t*)malloc(room.num_portals * sizeof(tr_room_portal_t));
    for (i = 0; i < room.num_portals; i++)
        read_tr_room_portal(newsrc, room.portals[i]);

    SDL_RWseek(newsrc, 224 + static_meshes_offset, SEEK_SET);

    room.static_meshes = (tr2_room_staticmesh_t*)malloc(room.num_static_meshes * sizeof(tr2_room_staticmesh_t));
    for (i = 0; i < room.num_static_meshes; i++)
        read_tr4_room_staticmesh(newsrc, room.static_meshes[i]);

    SDL_RWseek(newsrc, 224 + layer_offset, SEEK_SET);

    room.layers = (tr5_room_layer_t*)malloc(room.num_layers * sizeof(tr5_room_layer_t));
    for (i = 0; i < room.num_layers; i++)
        read_tr5_room_layer(newsrc, room.layers[i]);

    SDL_RWseek(newsrc, 224 + poly_offset, SEEK_SET);

    {
        uint32_t vertex_index = 0;
        uint32_t rectangle_index = 0;
        uint32_t triangle_index = 0;

        room.rectangles = (tr4_face4_t*)malloc(room.num_rectangles * sizeof(tr4_face4_t));
        room.triangles = (tr4_face3_t*)malloc(room.num_triangles * sizeof(tr4_face3_t));
        for (i = 0; i < room.num_layers; i++) {
            uint32_t j;

            for (j = 0; j < room.layers[i].num_rectangles; j++) {
                read_tr4_face4(newsrc, room.rectangles[rectangle_index]);
                room.rectangles[rectangle_index].vertices[0] += vertex_index;
                room.rectangles[rectangle_index].vertices[1] += vertex_index;
                room.rectangles[rectangle_index].vertices[2] += vertex_index;
                room.rectangles[rectangle_index].vertices[3] += vertex_index;
                rectangle_index++;
            }
            for (j = 0; j < room.layers[i].num_triangles; j++) {
                read_tr4_face3(newsrc, room.triangles[triangle_index]);
                room.triangles[triangle_index].vertices[0] += vertex_index;
                room.triangles[triangle_index].vertices[1] += vertex_index;
                room.triangles[triangle_index].vertices[2] += vertex_index;
                triangle_index++;
            }
            vertex_index += room.layers[i].num_vertices;
        }
    }

    SDL_RWseek(newsrc, 224 + vertices_offset, SEEK_SET);

    {
        uint32_t vertex_index = 0;
        room.num_vertices = vertices_size / 32;
        //int temp1 = room_data_size - (224 + vertices_offset + vertices_size);
        room.vertices = (tr5_room_vertex_t*)calloc(room.num_vertices, sizeof(tr5_room_vertex_t));
        for (i = 0; i < room.num_layers; i++) {
            uint32_t j;

            for (j = 0; j < room.layers[i].num_vertices; j++)
                    read_tr5_dc_room_vertex(newsrc, room.vertices[vertex_index++]);
        }
    }

    SDL_RWseek(newsrc, room_data_size, SEEK_SET);

    SDL_RWclose(newsrc);
    newsrc = NULL;
    delete [] buffer;
}

void TR_Level::read_tr5_dc_level(SDL_RWops * const src)
{
    uint32_t i;

    this->num_textiles = 0;
    this->num_room_textiles = 0;
    this->num_obj_textiles = 0;
    this->num_bump_textiles = 0;
    this->num_misc_textiles = 0;
    this->read_32bit_textiles = false;

    //Read header
    int offsetRoomStart = read_bitu32(src);

    //Read in the texture info
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);
    this->num_room_textiles = read_bitu32(src);
    this->num_obj_textiles = read_bitu32(src);
    this->num_bump_textiles = read_bitu32(src);
    this->num_misc_textiles = 3;//???
    this->num_textiles = this->num_room_textiles + this->num_obj_textiles + this->num_bump_textiles + this->num_misc_textiles;
    this->read_32bit_textiles = false;

    //Read textile16
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);
    this->textile16_count = this->num_textiles;
    this->textile16 = (tr2_textile16_t*)malloc(this->textile16_count * sizeof(tr2_textile16_t));
    for (i = 0; i < (this->num_textiles - this->num_misc_textiles); i++)
        read_tr2_textile16(src, this->textile16[i]);

    ///@FIXME: Unknown texture format!
    //Read misc textiles?
    SDL_RWseek(src, (this->num_misc_textiles * 0x4814), SEEK_CUR);

    //Read misc textile info?
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);
    SDL_RWseek(src, 0x4, SEEK_CUR);///@FIXME: What is this?
    //this->num_misc_textiles = read_bitu16(src);

    //Read Rooms
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

     //Unused
    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for 'unused'");

    this->rooms_count = read_bitu32(src);
    this->rooms = (tr5_room_t*)calloc(this->rooms_count, sizeof(tr5_room_t));
    for (i = 0; i < this->rooms_count; i++)
        read_tr5_dc_room(src, this->rooms[i]);

    //Read Floor Data
    this->floor_data_size = read_bitu32(src);
    this->floor_data = (uint16_t*)malloc(this->floor_data_size * sizeof(uint16_t));
    SDL_RWread(src, this->floor_data, this->floor_data_size * sizeof(uint16_t), 1);

    //Read Meshes
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);
    read_mesh_data(src);

    //Read Animations
    this->animations_count = read_bitu32(src);
    this->animations = (tr_animation_t*)malloc(this->animations_count * sizeof(tr_animation_t));
    for (i = 0; i < this->animations_count; i++)
    {
        read_tr4_animation(src, this->animations[i]);
    }

    //Read state changes
    this->state_changes_count = read_bitu32(src);
    this->state_changes = (tr_state_change_t*)malloc(this->state_changes_count * sizeof(tr_state_change_t));
    for (i = 0; i < this->state_changes_count; i++)
        read_tr_state_changes(src, this->state_changes[i]);

    //Read Anim Dispatches
    this->anim_dispatches_count = read_bitu32(src);
    this->anim_dispatches = (tr_anim_dispatch_t*)malloc(this->anim_dispatches_count * sizeof(tr_anim_dispatch_t));
    for (i = 0; i < this->anim_dispatches_count; i++)
        read_tr_anim_dispatches(src, this->anim_dispatches[i]);

    //Read Anim Commands
    this->anim_commands_count = read_bitu32(src);
    this->anim_commands = (int16_t*)malloc(this->anim_commands_count * sizeof(int16_t));
    SDL_RWread(src, this->anim_commands, this->anim_commands_count * sizeof(uint16_t), 1);

    //Read Mesh Trees
    this->mesh_tree_data_size = read_bitu32(src);
    this->mesh_tree_data = (uint32_t*)malloc(this->mesh_tree_data_size * sizeof(uint32_t));
    SDL_RWread(src, this->mesh_tree_data, this->mesh_tree_data_size * sizeof(uint32_t), 1);

    //Read Frame Movable Data
    read_frame_moveable_data(src);

    //Read Static Meshes
    this->static_meshes_count = read_bitu32(src);
    this->static_meshes = (tr_staticmesh_t*)malloc(this->static_meshes_count * sizeof(tr_staticmesh_t));
    for (i = 0; i < this->static_meshes_count; i++)
        read_tr_staticmesh(src, this->static_meshes[i]);

    //Read Sprites
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    if (read_bitu32(src) != 0x00525053)
        Sys_extError("read_tr5_dc_level: 'SPR\0' not found");

    this->sprite_textures_count = read_bitu32(src);
    this->sprite_textures = (tr_sprite_texture_t*)malloc(this->sprite_textures_count * sizeof(tr_sprite_texture_t));
    for (i = 0; i < this->sprite_textures_count; i++)
        read_tr4_sprite_texture(src, this->sprite_textures[i]);

    //Read Sprite Sequences
    this->sprite_sequences_count = read_bitu32(src);
    this->sprite_sequences = (tr_sprite_sequence_t*)malloc(this->sprite_sequences_count * sizeof(tr_sprite_sequence_t));
    SDL_RWread(src, this->sprite_sequences, this->sprite_sequences_count * sizeof(tr_sprite_sequence_t), 1);

    //Read Cameras
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    this->cameras_count = read_bitu32(src);
    this->cameras = (tr_camera_t*)malloc(this->cameras_count * sizeof(tr_camera_t));
    for (i = 0; i < this->cameras_count; i++)
    {
        this->cameras[i].x = read_bit32(src);
        this->cameras[i].y = read_bit32(src);
        this->cameras[i].z = read_bit32(src);

        this->cameras[i].room = read_bit16(src);
        this->cameras[i].unknown1 = read_bitu16(src);
    }

    //Read flyby cameras
    this->flyby_cameras_count = read_bitu32(src);
    this->flyby_cameras = (tr4_flyby_camera_t*)malloc(this->flyby_cameras_count * sizeof(tr4_flyby_camera_t));
    for (i = 0; i < this->flyby_cameras_count; i++)
    {
        this->flyby_cameras[i].x1 = read_bit32(src);
        this->flyby_cameras[i].y1 = read_bit32(src);
        this->flyby_cameras[i].z1 = read_bit32(src);
        this->flyby_cameras[i].x2 = read_bit32(src);
        this->flyby_cameras[i].y2 = read_bit32(src);
        this->flyby_cameras[i].z2 = read_bit32(src);                    // 24

        this->flyby_cameras[i].index1 = read_bit8(src);
        this->flyby_cameras[i].index2 = read_bit8(src);                 // 26

        this->flyby_cameras[i].unknown[0] = read_bitu16(src);
        this->flyby_cameras[i].unknown[1] = read_bitu16(src);
        this->flyby_cameras[i].unknown[2] = read_bitu16(src);
        this->flyby_cameras[i].unknown[3] = read_bitu16(src);
        this->flyby_cameras[i].unknown[4] = read_bitu16(src);           // 36

        this->flyby_cameras[i].id = read_bit32(src);                    // 40
    }

    //Read Sound sources
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    this->sound_sources_count = read_bitu32(src);
    this->sound_sources = (tr_sound_source_t*)malloc(this->sound_sources_count * sizeof(tr_sound_source_t));
    for(i = 0; i < this->sound_sources_count; i++)
    {
        this->sound_sources[i].x = read_bit32(src);
        this->sound_sources[i].y = read_bit32(src);
        this->sound_sources[i].z = read_bit32(src);

        this->sound_sources[i].sound_id = read_bitu16(src);
        this->sound_sources[i].flags = read_bitu16(src);
    }

    //Read Boxes
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    this->boxes_count = read_bitu32(src);
    this->boxes = (tr_box_t*)malloc(this->boxes_count * sizeof(tr_box_t));
    for (i = 0; i < this->boxes_count; i++)
        read_tr2_box(src, this->boxes[i]);

    //Read Overlaps
    this->overlaps_count = read_bitu32(src);
    this->overlaps = (uint16_t*)malloc(this->overlaps_count * sizeof(uint16_t));
    for (i = 0; i < this->overlaps_count; i++)
        this->overlaps[i] = read_bitu16(src);

    ///Skip Zones
    SDL_RWseek(src, this->boxes_count * 20, SEEK_CUR);

    //Read Animated Textures
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    this->animated_textures_count = read_bitu32(src);
    this->animated_textures = (uint16_t*)malloc(this->animated_textures_count * sizeof(uint16_t));
    for (i = 0; i < this->animated_textures_count; i++)
    {
        this->animated_textures[i] = read_bitu16(src);
    }

     this->animated_textures_uv_count = read_bitu8(src);

    //Read Object Textures
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    if (read_bitu32(src) != 0x00584554)
        Sys_extError("read_tr5_dc_level: 'TEX\0' not found");

    this->object_textures_count = read_bitu32(src);
    this->object_textures = (tr4_object_texture_t*)malloc(this->object_textures_count * sizeof(tr4_object_texture_t));
    for (i = 0; i < this->object_textures_count; i++)
    {
        read_tr4_object_texture(src, this->object_textures[i]);
        if (read_bitu16(src) != 0)
            Sys_extWarn("read_tr5_dc_level: obj_tex trailing bitu16 != 0");
    }

    //Read Items
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    this->items_count = read_bitu32(src);
    this->items = (tr2_item_t*)malloc(this->items_count * sizeof(tr2_item_t));
    for (i = 0; i < this->items_count; i++)
        read_tr4_item(src, this->items[i]);

    //Read AI Objects
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    this->ai_objects_count = read_bitu32(src);
    this->ai_objects = (tr4_ai_object_t*)malloc(this->ai_objects_count * sizeof(tr4_ai_object_t));
    for(i=0; i < this->ai_objects_count; i++)
    {
        this->ai_objects[i].object_id = read_bitu16(src);
        this->ai_objects[i].room = read_bitu16(src);

        this->ai_objects[i].x = read_bit32(src);
        this->ai_objects[i].y = read_bit32(src);
        this->ai_objects[i].z = read_bit32(src);                            // 16

        this->ai_objects[i].ocb = read_bitu16(src);
        this->ai_objects[i].flags = read_bitu16(src);                       // 20
        this->ai_objects[i].angle = read_bit32(src);                        // 24
    }

    //Read Demo Data
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    this->demo_data_count = read_bitu16(src);
    this->demo_data = (uint8_t*)malloc(this->demo_data_count * sizeof(uint8_t));
    SDL_RWread(src, this->demo_data, this->demo_data_count * sizeof(uint8_t), 1);

    //Read Sound Map
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    this->soundmap = (int16_t*)malloc(TR_AUDIO_MAP_SIZE_TR5 * sizeof(int16_t));
    SDL_RWread(src, this->soundmap, TR_AUDIO_MAP_SIZE_TR5 * sizeof(int16_t), 1);

    //Read Sound Details
    this->sound_details_count = read_bitu32(src);
    this->sound_details = (tr_sound_details_t*)malloc(this->sound_details_count * sizeof(tr_sound_details_t));

    for(i=0; i < this->sound_details_count; i++)
    {
        this->sound_details[i].sample = read_bitu16(src);
        this->sound_details[i].volume = (uint16_t)read_bitu8(src);        // n x 2.6
        this->sound_details[i].sound_range = (uint16_t)read_bitu8(src);   // n as is
        this->sound_details[i].chance = (uint16_t)read_bitu8(src);        // If n = 99, n = 0 (max. chance)
        this->sound_details[i].pitch = (int16_t)read_bit8(src);           // n as is
        this->sound_details[i].num_samples_and_flags_1 = read_bitu8(src);
        this->sound_details[i].flags_2 = read_bitu8(src);
    }

    //Read Sample Indices
    this->sample_indices_count = read_bitu32(src);
    this->sample_indices = (uint32_t*)malloc(this->sample_indices_count * sizeof(uint32_t));
    SDL_RWread(src, this->sample_indices, this->sample_indices_count * sizeof(uint32_t), 1);

    //Read Samples Data Info
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    if (read_bitu32(src) != 0x504D4153)
        Sys_extError("read_tr5_dc_level: 'SAMP' not found");

    uint32_t sample_data_length = read_bitu32(src);

    //Read Samples
    SDL_RWseek(src, ((SDL_RWtell(src) + 0x7FF) & ~0x7FF), SEEK_SET);

    if (read_bitu32(src) != 0x53534F54)
        Sys_extError("read_tr5_dc_level: 'TOSS' not found");

    this->samples_count = read_bitu32(src);
    ///@FIXME: Dreamcast WAV files are Yamaha ACPDM encoded. they are NOT IMA ADPCM and therefore will NOT play unless converted!
    this->samples_count = 0;

    //Read block of sound sample data into singular array
    this->samples_data_size = sample_data_length;
    this->samples_data = (uint8_t*)malloc(this->samples_data_size * sizeof(uint8_t));
    SDL_RWread(src, this->samples_data, this->samples_data_size, 1);
    //Sys_extError("Current Offset %i", SDL_RWtell(src));
}
