/*=============================================================================
 * This file is part of the "VirtualMuseum" application
 * Copyright (C) Iftode Bogdan-Marius <iftode.bogdan@gmail.com>
 *
 *      File:   SceneData.cpp
 *      Author: Bogdan Iftode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
=============================================================================*/

#include "stdafx.h"

#include "SceneData.h"
#include "AppResources.h"
using namespace VirtualMuseumApp;

const char Room1Name[] = { 0x53, 0x75, 0x70, 0x65, 0x72, 0x20, 0x4D, 0x61, 0x72, 0x69, 0x6F };
const char Room2Name[] = { 0x43, 0x65, 0x6C, 0x65, 0x20, 0x6D, 0x61, 0x69, 0x20, 0x69, 0x6E, 0x64, 0x72, 0x61, 0x67, 0x69, 0x74, 0x65, 0x20, 0x73, 0x65, 0x72, 0x69, 0x69, 0x20, 0x64, 0x65, 0x20, 0x6A, 0x6F, 0x63, 0x75, 0x72, 0x69, 0x20, 0x76, 0x69, 0x64, 0x65, 0x6F, 0x20, 0x28, 0x49, 0x29, 0x3A, 0x20, 0x46, 0x69, 0x6E, 0x61, 0x6C, 0x20, 0x46, 0x61, 0x6E, 0x74, 0x61, 0x73, 0x79, 0x2C, 0x20, 0x4B, 0x69, 0x6E, 0x67, 0x64, 0x6F, 0x6D, 0x20, 0x48, 0x65, 0x61, 0x72, 0x74, 0x73 };
const char Room3Name[] = { 0x43, 0x65, 0x6C, 0x65, 0x20, 0x6D, 0x61, 0x69, 0x20, 0x69, 0x6E, 0x64, 0x72, 0x61, 0x67, 0x69, 0x74, 0x65, 0x20, 0x73, 0x65, 0x72, 0x69, 0x69, 0x20, 0x64, 0x65, 0x20, 0x6A, 0x6F, 0x63, 0x75, 0x72, 0x69, 0x20, 0x76, 0x69, 0x64, 0x65, 0x6F, 0x20, 0x28, 0x49, 0x49, 0x29, 0x3A, 0x20, 0x47, 0x6F, 0x64, 0x20, 0x6F, 0x66, 0x20, 0x57, 0x61, 0x72, 0x2C, 0x20, 0x42, 0x69, 0x6F, 0x73, 0x68, 0x6F, 0x63, 0x6B };
const char Room4Name[] = { 0x45, 0x70, 0x6F, 0x63, 0x61, 0x20, 0x64, 0x65, 0x20, 0x61, 0x75, 0x72, 0x20, 0x61, 0x20, 0x6A, 0x6F, 0x63, 0x75, 0x72, 0x69, 0x6C, 0x6F, 0x72, 0x20, 0x61, 0x72, 0x63, 0x61, 0x64, 0x65, 0x3A, 0x20, 0x50, 0x61, 0x63, 0x2D, 0x4D, 0x61, 0x6E, 0x2C, 0x20, 0x44, 0x6F, 0x6E, 0x6B, 0x65, 0x79, 0x20, 0x4B, 0x6F, 0x6E, 0x67 };
const char Room5Name[] = { 0x49, 0x6E, 0x74, 0x72, 0x2D, 0x75, 0x6E, 0x20, 0x75, 0x6E, 0x69, 0x76, 0x65, 0x72, 0x73, 0x20, 0x69, 0x6E, 0x74, 0x72, 0x6F, 0x73, 0x70, 0x65, 0x63, 0x74, 0x69, 0x76, 0x3A, 0x20, 0x46, 0x6C, 0x6F, 0x72, 0x65, 0x6E, 0x63, 0x65, 0x2C, 0x20, 0x45, 0x76, 0x65, 0x72, 0x79, 0x62, 0x6F, 0x64, 0x79, 0x27, 0x73, 0x20, 0x47, 0x6F, 0x6E, 0x65, 0x20, 0x74, 0x6F, 0x20, 0x74, 0x68, 0x65, 0x20, 0x52, 0x61, 0x70, 0x74, 0x75, 0x72, 0x65 };

const char Room1Desc[] = "test";

static SceneData g_SceneData[] =
{
    {
        "sounds/hall1.wav",
        8, { &Room1_1, &Room1_2, &Room1_3, &Room1_4, &Room1_5, &Room1_6, &Room1_7, &Room1_8 },
        { Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc },
        Room1Name
    },

    {
        "sounds/hall2.wav",
        8, { &Room2_1, &Room2_2, &Room2_3, &Room2_4, &Room2_5, &Room2_6, &Room2_7, &Room2_8 },
        { Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc },
        Room2Name
    },

    {
        "sounds/hall3.wav",
        8, { &Room3_1, &Room3_2, &Room3_3, &Room3_4, &Room3_5, &Room3_6, &Room3_7, &Room3_8 },
        { Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc },
        Room3Name
    },

    {
        "sounds/hall4.wav",
        8, { &Room4_1, &Room4_2, &Room4_3, &Room4_4, &Room4_5, &Room4_6, &Room4_7, &Room4_8 },
        { Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc },
        Room4Name
    },

    {
        "sounds/hall5.wav",
        8, { &Room5_1, &Room5_2, &Room5_3, &Room5_4, &Room5_5, &Room5_6, &Room5_7, &Room5_8 },
        { Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc, Room1Desc },
        Room5Name
    },
};

SceneData* SceneData::m_sSceneData = g_SceneData;
