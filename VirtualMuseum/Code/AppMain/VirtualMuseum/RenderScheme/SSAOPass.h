/*=============================================================================
 * This file is part of the "VirtualMuseum" application
 * Copyright (C) Iftode Bogdan-Marius <iftode.bogdan@gmail.com>
 *
 *      File:   SSAOPass.h
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

#ifndef SSAO_PASS_H_
#define SSAO_PASS_H_

#include "RenderPass.h"

namespace VirtualMuseumApp
{
    class RenderTarget;

    class SSAOPass : public RenderPass
    {
        IMPLEMENT_RENDER_PASS(SSAOPass)

    private:
        void CalculateSSAO();
        void BlurSSAO();
        void ApplySSAO();

        RenderTarget**  SSAOBuffer;
        unsigned int    BlurKernelCount;
    };
}

#endif //SSAO_PASS_H_