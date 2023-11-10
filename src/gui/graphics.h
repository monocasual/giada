/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * graphics
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifndef G_GUI_GRAPHICS_H
#define G_GUI_GRAPHICS_H

namespace giada::v::graphics
{
constexpr auto giadaLogo = "<svg width=\"300\" height=\"104.75\" version=\"1.1\" viewBox=\"0 0 85.725 29.934\"><g transform=\"matrix(.7569 0 0 .7569 -20.373 -42.578)\" clip-rule=\"evenodd\" fill-rule=\"evenodd\" shape-rendering=\"geometricPrecision\"><g transform=\"matrix(.0254 0 0 .0254 -14.105 -43.849)\" image-rendering=\"optimizeQuality\" stroke-width=\"2.3061\"><path class=\"fil1\" d=\"m2394 3941c430 0 778 348 778 779 0 430-348 778-778 778-431 0-779-348-779-778 0-431 348-779 779-779z\" fill=\"#454545\"/><path class=\"fil2\" d=\"m2394 4362c197 0 357 160 357 358v2h11c91 0 171 43 222 110 7-36 10-74 10-112 0-332-269-601-600-601-332 0-601 269-601 601 0 331 269 600 601 600 89 0 175-20 251-55 27 10 56 15 86 15h17c126-9 227-114 227-243 0-48-14-92-37-130-44-68-120-114-207-114-135 0-244 109-244 244 0 9 0 18 1 27-30 8-62 13-94 13-198 0-358-160-358-357 0-198 160-358 358-358zm151 263c-32-50-88-84-151-84-99 0-179 80-179 179 0 98 80 178 179 178 4 0 8 0 12-1 93-6 166-83 166-177 0-35-10-68-27-95z\" fill=\"#fefefe\"/></g><path class=\"fil1\" d=\"m78.934 83.836c-2.1082 0-3.7338-0.508-4.9276-1.524-1.1938-1.0414-1.8034-2.4892-1.8796-4.3688-0.0254-0.4064-0.0254-1.0668-0.0254-1.9812 0-0.91439 0-1.5748 0.0254-2.0066 0.0508-1.8034 0.6858-3.2258 1.905-4.2164 1.1938-1.016 2.8194-1.524 4.9022-1.524 1.3716 0 2.5908 0.2286 3.6322 0.6858 1.016 0.4318 1.8034 1.016 2.3622 1.7272 0.5334 0.6858 0.81279 1.4224 0.83819 2.159 0 0.127-0.0254 0.2286-0.127 0.3302-0.1016 0.0762-0.2032 0.127-0.3302 0.127h-3.3782c-0.127 0-0.254-0.0254-0.3302-0.0762s-0.1524-0.127-0.2286-0.2286c-0.1778-0.4064-0.4572-0.7366-0.83819-1.0414-0.381-0.2794-0.91439-0.4318-1.6002-0.4318-1.6764 0-2.54 0.86359-2.6162 2.6162v3.7338c0.0762 1.8542 0.93979 2.7686 2.6416 2.7686 0.83819 0 1.524-0.2286 2.0066-0.635 0.4826-0.4318 0.7366-1.0668 0.7366-1.905v-0.381h-1.9304c-0.1524 0-0.2794-0.0762-0.4064-0.1778-0.1016-0.1016-0.1524-0.2286-0.1524-0.4064v-1.6764c0-0.1524 0.0508-0.3048 0.1524-0.4064 0.127-0.1016 0.254-0.1524 0.4064-0.1524h5.5118c0.1524 0 0.2794 0.0508 0.4064 0.1524 0.1016 0.1016 0.1524 0.254 0.1524 0.4064v2.5146c0 1.2192-0.2794 2.286-0.86359 3.175-0.5588 0.88899-1.3716 1.5494-2.413 2.032-1.0414 0.4572-2.2606 0.7112-3.6322 0.7112zm9.8551-0.2286c-0.1524 0-0.2794-0.0508-0.381-0.1524-0.1016-0.127-0.1524-0.2286-0.1524-0.381v-14.097c0-0.1524 0.0508-0.2794 0.1524-0.381s0.2286-0.1524 0.381-0.1524h3.0988c0.1524 0 0.2794 0.0508 0.381 0.1524s0.1524 0.2286 0.1524 0.381v14.097c0 0.1524-0.0508 0.254-0.1524 0.381-0.1016 0.1016-0.2286 0.1524-0.381 0.1524zm5.7404 0c-0.1016 0-0.2286-0.0508-0.3048-0.127-0.1016-0.1016-0.1524-0.2032-0.1524-0.3302 0-0.0762 0-0.1524 0.0254-0.1778l5.0546-13.894c0.127-0.4318 0.4318-0.635 0.8636-0.635h3.3782c0.4318 0 0.7112 0.2032 0.8636 0.635l5.0546 13.894c0.0254 0.0254 0.0254 0.1016 0.0254 0.1778 0 0.127-0.0508 0.2286-0.1524 0.3302-0.0762 0.0762-0.2032 0.127-0.3048 0.127h-2.8194c-0.35559 0-0.58419-0.1524-0.73659-0.4826l-0.7874-2.0828h-5.6642l-0.78739 2.0828c-0.1524 0.3302-0.381 0.4826-0.7366 0.4826zm5.2578-5.8166h3.8354l-1.9304-5.5372zm11.735 5.8166c-0.1524 0-0.2794-0.0508-0.381-0.1524-0.1016-0.127-0.1524-0.2286-0.1524-0.381v-14.071c0-0.1524 0.0508-0.2794 0.1524-0.4064 0.1016-0.1016 0.2286-0.1524 0.381-0.1524h5.6896c2.159 0 3.81 0.508 5.0038 1.524 1.1684 1.016 1.8034 2.4892 1.8542 4.3942 0.0254 0.4064 0.0254 0.96519 0.0254 1.6764 0 0.6858 0 1.2192-0.0254 1.6256-0.1524 3.9624-2.3876 5.9436-6.731 5.9436zm5.6896-3.2512c0.91439 0 1.6002-0.2032 2.032-0.635s0.6604-1.143 0.6858-2.0828c0.0254-0.4318 0.0508-0.96519 0.0508-1.6256 0-0.6604-0.0254-1.2192-0.0508-1.6002-0.0254-0.93979-0.2794-1.6256-0.7366-2.0574-0.4826-0.4318-1.1684-0.6604-2.0828-0.6604h-2.0574v8.6613zm8.1533 3.2512c-0.1016 0-0.2286-0.0508-0.3048-0.127-0.1016-0.1016-0.1524-0.2032-0.1524-0.3302 0-0.0762 0.0254-0.1524 0.0254-0.1778l5.0546-13.894c0.1524-0.4318 0.4318-0.635 0.8636-0.635h3.3782c0.4318 0 0.7366 0.2032 0.8636 0.635l5.0546 13.894c0.0254 0.0254 0.0254 0.1016 0.0254 0.1778 0 0.127-0.0508 0.2286-0.1524 0.3302-0.0762 0.0762-0.1778 0.127-0.30479 0.127h-2.8194c-0.3556 0-0.5842-0.1524-0.7366-0.4826l-0.78739-2.0828h-5.6642l-0.7874 2.0828c-0.1524 0.3302-0.381 0.4826-0.73659 0.4826zm5.2578-5.8166h3.8608l-1.9304-5.5372z\" fill=\"#454545\" image-rendering=\"optimizeQuality\" stroke-width=\".033558\"/></g></svg>";
constexpr auto giadaIcon = "<svg width=\"65\" height=\"65\" version=\"1.1\" viewBox=\"0 0 17.198 17.198\"><g transform=\"translate(-15.032 -42.184)\"><g transform=\"matrix(.011046 0 0 .011046 -2.8069 -1.347)\" clip-rule=\"evenodd\" fill-rule=\"evenodd\" image-rendering=\"optimizeQuality\" shape-rendering=\"geometricPrecision\" stroke-width=\"7.3846\"><path class=\"fil1\" d=\"m2394 3941c430 0 778 348 778 779 0 430-348 778-778 778-431 0-779-348-779-778 0-431 348-779 779-779z\" fill=\"#454545\"/><path class=\"fil2\" d=\"m2394 4362c197 0 357 160 357 358v2h11c91 0 171 43 222 110 7-36 10-74 10-112 0-332-269-601-600-601-332 0-601 269-601 601 0 331 269 600 601 600 89 0 175-20 251-55 27 10 56 15 86 15h17c126-9 227-114 227-243 0-48-14-92-37-130-44-68-120-114-207-114-135 0-244 109-244 244 0 9 0 18 1 27-30 8-62 13-94 13-198 0-358-160-358-357 0-198 160-358 358-358zm151 263c-32-50-88-84-151-84-99 0-179 80-179 179 0 98 80 178 179 178 4 0 8 0 12-1 93-6 166-83 166-177 0-35-10-68-27-95z\" fill=\"#fefefe\"/></g></g></svg>";

constexpr auto playOff   = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(-.13229 -.13229)\"><rect x=\".13229\" y=\".13229\" width=\"6.0854\" height=\"6.0854\" color=\"#000000\" fill=\"#252525\"/><path d=\"m1.7198 1.463 2.9104 1.712-2.9104 1.712z\" color=\"#000000\" fill=\"#b18e8e\"/></g></svg>";
constexpr auto playOn    = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(-.13229 -.13229)\"><rect x=\".13229\" y=\".13229\" width=\"6.0854\" height=\"6.0854\" color=\"#000000\" fill=\"#4e4e4e\"/><path d=\"m1.7198 1.463 2.9104 1.712-2.9104 1.712z\" color=\"#000000\" fill=\"#ae53d0\"/></g></svg>";
constexpr auto rewindOff = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(-.13229 -.13229)\"><rect x=\".13229\" y=\".13229\" width=\"6.0854\" height=\"6.0854\" color=\"#000000\" fill=\"#252525\"/><path d=\"m4.6302 1.463-2.9104 1.712 2.9104 1.712z\" color=\"#000000\" fill=\"#b18e8e\"/></g></svg>";
constexpr auto rewindOn  = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(-.13229 -.13229)\"><rect x=\".13229\" y=\".13229\" width=\"6.0854\" height=\"6.0854\" color=\"#000000\" fill=\"#4e4e4e\"/><path d=\"m4.6302 1.463-2.9104 1.712 2.9104 1.712z\" color=\"#000000\" fill=\"#ae53d0\"/></g></svg>";

constexpr auto recTriggerModeOff = "<svg width=\"13\" height=\"23\" version=\"1.1\" viewBox=\"0 0 3.4396 6.0854\"><g transform=\"translate(-.13229 -.13229)\"><rect x=\".13229\" y=\".13229\" width=\"3.4396\" height=\"6.0854\" color=\"#000000\" fill=\"#252525\"/><g transform=\"matrix(.26458 0 0 .26458 -23.151 -147.97)\" fill=\"#b18e8e\"><circle cx=\"94.5\" cy=\"576.02\" r=\"1.5\" color=\"#000000\"/><circle cx=\"94.5\" cy=\"566.52\" r=\"1.5\" color=\"#000000\"/><circle cx=\"94.5\" cy=\"571.27\" r=\"1.5\" color=\"#000000\"/></g></g></svg>";
constexpr auto recTriggerModeOn  = "<svg width=\"13\" height=\"23\" version=\"1.1\" viewBox=\"0 0 3.4396 6.0854\"><g transform=\"translate(-.13229 -.13229)\"><rect x=\".13229\" y=\".13229\" width=\"3.4396\" height=\"6.0854\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"matrix(.26458 0 0 .26458 -23.151 -147.97)\" fill=\"#b18e8e\"><circle cx=\"94.5\" cy=\"576.02\" r=\"1.5\" color=\"#000000\"/><circle cx=\"94.5\" cy=\"566.52\" r=\"1.5\" color=\"#000000\"/><circle cx=\"94.5\" cy=\"571.27\" r=\"1.5\" color=\"#000000\"/></g></g></svg>";

constexpr auto actionRecOff = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(3.9043 -.024934)\"><rect x=\"-3.9043\" y=\".024934\" width=\"6.0854\" height=\"6.0854\" color=\"#000000\" fill=\"#252525\"/><path d=\"m-0.86155 1.4801c-0.87675 0-1.5875 0.71075-1.5875 1.5875s0.71075 1.5875 1.5875 1.5875 1.5875-0.71075 1.5875-1.5875-0.71075-1.5875-1.5875-1.5875zm0 0.79375c0.43838 0 0.79375 0.35537 0.79375 0.79375s-0.35537 0.79375-0.79375 0.79375c-0.43838 0-0.79375-0.35537-0.79375-0.79375s0.35537-0.79375 0.79375-0.79375z\" color=\"#000000\" fill=\"#b18e8e\"/></g></svg>";
constexpr auto actionRecOn  = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(5.4333 1.8043)\"><rect x=\"-5.301\" y=\"-1.6721\" width=\"5.8208\" height=\"5.8208\" color=\"#000000\" fill=\"#4e4e4e\" stroke=\"#4e4e4e\"/><path d=\"m-2.3906-0.34914c-0.87675 0-1.5875 0.71075-1.5875 1.5875 0 0.87675 0.71075 1.5875 1.5875 1.5875 0.87675 0 1.5875-0.71075 1.5875-1.5875 0-0.87675-0.71075-1.5875-1.5875-1.5875zm0 0.79375c0.43838 0 0.79375 0.35537 0.79375 0.79375 0 0.43838-0.35537 0.79375-0.79375 0.79375-0.43838 0-0.79375-0.35537-0.79375-0.79375 0-0.43838 0.35537-0.79375 0.79375-0.79375z\" color=\"#000000\" fill=\"#ea5555\"/></g></svg>";

constexpr auto inputRecOff      = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(4.1552 -3.9773)\"><rect x=\"-4.1552\" y=\"3.9773\" width=\"6.0854\" height=\"6.0854\" color=\"#000000\" fill=\"#252525\"/><ellipse cx=\"-1.1125\" cy=\"7.02\" rx=\"1.5875\" ry=\"1.5875\" color=\"#000000\" fill=\"#b18e8e\"/></g></svg>";
constexpr auto inputRecOn       = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(8.5953 3.7262)\"><rect x=\"-8.5953\" y=\"-3.7262\" width=\"6.0854\" height=\"6.0854\" color=\"#000000\" fill=\"#4e4e4e\"/><ellipse cx=\"-5.5525\" cy=\"-.68348\" rx=\"1.5875\" ry=\"1.5875\" color=\"#000000\" fill=\"#ea5555\"/></g></svg>";
constexpr auto inputRecDisabled = "<svg width=\"23\" height=\"23\" version=\"1.1\" viewBox=\"0 0 6.0854 6.0854\"><g transform=\"translate(8.5953 3.7262)\"><rect x=\"-8.5953\" y=\"-3.7262\" width=\"6.0854\" height=\"6.0854\" color=\"#000000\" fill=\"#252525\"/><circle cx=\"-5.5525\" cy=\"-.68348\" r=\"1.5875\" color=\"#000000\" fill=\"#4e4e4e\"/></g></svg>";

constexpr auto freeInputRecOff      = "<svg width=\"13\" height=\"23\" version=\"1.1\" viewBox=\"0 0 3.4396 6.0854\"><g transform=\"translate(5.9681 .63361)\"><rect x=\"-5.9681\" y=\"-.63361\" width=\"3.4396\" height=\"6.0854\" color=\"#000000\" fill=\"#252525\"/><g transform=\"matrix(.26458 0 0 .26458 -49.757 -148.9)\" fill=\"#b18e8e\"><rect x=\"171\" y=\"565.92\" width=\"2\" height=\"4\"/><rect x=\"171\" y=\"573.82\" width=\"2\" height=\"4\"/><rect transform=\"matrix(.54861 -.83608 .89156 .45291 0 0)\" x=\"-435.43\" y=\"457.45\" width=\"1.6359\" height=\"5.8119\"/></g></g></svg>";
constexpr auto freeInputRecOn       = "<svg width=\"13\" height=\"23\" version=\"1.1\" viewBox=\"0 0 3.4396 6.0854\"><g transform=\"translate(7.0272 3.5725)\"><rect x=\"-7.0272\" y=\"-3.5725\" width=\"3.4396\" height=\"6.0854\" color=\"#000000\" fill=\"#4e4e4e\" stop-color=\"#000000\"/><g transform=\"matrix(.26458 0 0 .26458 -50.816 -151.84)\" fill=\"#b18e8e\"><rect x=\"171\" y=\"565.92\" width=\"2\" height=\"4\"/><rect x=\"171\" y=\"573.82\" width=\"2\" height=\"4\"/><rect transform=\"matrix(.54861 -.83608 .89156 .45291 0 0)\" x=\"-435.43\" y=\"457.45\" width=\"1.6359\" height=\"5.8119\"/></g></g></svg>";
constexpr auto freeInputRecDisabled = "<svg width=\"13\" height=\"23\" version=\"1.1\" viewBox=\"0 0 13 23\"><g transform=\"translate(-78.552 -136.23)\"><g transform=\"translate(-113 -230.56)\"><rect x=\"191.55\" y=\"366.79\" width=\"13\" height=\"23\" color=\"#000000\" fill=\"#252525\"/><g transform=\"translate(26.049 -193.58)\" fill=\"#4e4e4e\"><rect x=\"171\" y=\"565.92\" width=\"2\" height=\"4\"/><rect x=\"171\" y=\"573.82\" width=\"2\" height=\"4\"/><rect transform=\"matrix(.54861 -.83608 .89156 .45291 0 0)\" x=\"-435.43\" y=\"457.45\" width=\"1.6359\" height=\"5.8119\"/></g></g></g></svg>";

constexpr auto metronomeOff = "<svg width=\"13\" height=\"23\" version=\"1.1\" viewBox=\"0 0 3.4396 6.0854\"><g transform=\"translate(2.7553 2.9993)\"><rect x=\"-2.7553\" y=\"-2.9993\" width=\"3.4396\" height=\"6.0854\" color=\"#000000\" fill=\"#252525\"/><g transform=\"translate(-.13229)\" fill=\"#b18e8e\"><rect x=\"-1.5647\" y=\"-1.2795\" width=\".26458\" height=\"2.6458\"/><rect x=\"-.50638\" y=\"-1.2795\" width=\".26458\" height=\"2.6458\"/></g></g></svg>";
constexpr auto metronomeOn  = "<svg width=\"13\" height=\"23\" version=\"1.1\" viewBox=\"0 0 3.4396 6.0854\"><g transform=\"translate(7.0272 3.5725)\"><rect x=\"-6.895\" y=\"-3.4402\" width=\"3.175\" height=\"5.8208\" color=\"#000000\" fill=\"#4e4e4e\" stroke=\"#4e4e4e\"/><g transform=\"translate(-3.7497 -3.7174)\" fill=\"#b18e8e\"><rect x=\"-2.2192\" y=\"1.8647\" width=\".26458\" height=\"2.6458\"/><rect x=\"-1.1608\" y=\"1.8647\" width=\".26458\" height=\"2.6458\"/></g></g></svg>";

constexpr auto fxOff = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-24.644 -89.039)\"><rect x=\"24.644\" y=\"89.039\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g fill=\"#c8c8c8\" aria-label=\"FX\"><path d=\"m25.361 90.552h1.2081v0.33835h-0.76042v0.32324h0.71507v0.33835h-0.71507v0.736h-0.44765z\"/><path d=\"m28.087 91.402 0.60229 0.88599h-0.46625l-0.40579-0.59299-0.4023 0.59299h-0.46858l0.60229-0.88599-0.57903-0.84995h0.46741l0.38021 0.55927 0.37905-0.55927h0.46974z\"/></g></g></svg>";
constexpr auto fxOn  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-24.644 -89.039)\"><rect x=\"24.644\" y=\"89.039\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g fill=\"#c8c8c8\" aria-label=\"FX\"><path d=\"m25.361 90.552h1.2081v0.33835h-0.76042v0.32324h0.71507v0.33835h-0.71507v0.736h-0.44765z\"/><path d=\"m28.087 91.402 0.60229 0.88599h-0.46625l-0.40579-0.59299-0.4023 0.59299h-0.46858l0.60229-0.88599-0.57903-0.84995h0.46741l0.38021 0.55927 0.37905-0.55927h0.46974z\"/></g></g></svg>";

constexpr auto readActionOff      = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-24.644 -89.039)\"><rect x=\"24.644\" y=\"89.039\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"translate(-3.5843 -2.7358)\" fill=\"#c8c8c8\" aria-label=\"R\"><path d=\"m30.413 94.025q0.18759 0 0.2682-0.06976 0.08216-0.06976 0.08216-0.22944 0-0.15813-0.08216-0.22634-0.08062-0.06821-0.2682-0.06821h-0.25115v0.59376zm-0.25115 0.41238v0.87592h-0.59686v-2.3146h0.91157q0.45734 0 0.66973 0.15348 0.21394 0.15348 0.21394 0.48524 0 0.22944-0.11162 0.37672-0.11007 0.14728-0.33331 0.21704 0.12247 0.0279 0.21859 0.12712 0.09767 0.09767 0.19689 0.29766l0.32401 0.65732h-0.63562l-0.28215-0.57516q-0.08527-0.17363-0.17363-0.2372-0.08682-0.06356-0.23254-0.06356z\"/></g></g></svg>";
constexpr auto readActionOn       = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-24.644 -89.039)\"><rect x=\"24.644\" y=\"89.039\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"translate(-3.5843 -2.7358)\" fill=\"#c8c8c8\" aria-label=\"R\"><path d=\"m30.413 94.025q0.18759 0 0.2682-0.06976 0.08216-0.06976 0.08216-0.22944 0-0.15813-0.08216-0.22634-0.08062-0.06821-0.2682-0.06821h-0.25115v0.59376zm-0.25115 0.41238v0.87592h-0.59686v-2.3146h0.91157q0.45734 0 0.66973 0.15348 0.21394 0.15348 0.21394 0.48524 0 0.22944-0.11162 0.37672-0.11007 0.14728-0.33331 0.21704 0.12247 0.0279 0.21859 0.12712 0.09767 0.09767 0.19689 0.29766l0.32401 0.65732h-0.63562l-0.28215-0.57516q-0.08527-0.17363-0.17363-0.2372-0.08682-0.06356-0.23254-0.06356z\"/></g></g></svg>";
constexpr auto readActionDisabled = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-24.644 -89.039)\"><rect x=\"24.644\" y=\"89.039\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"translate(-3.5843 -2.7358)\" fill=\"#5a5a5a\" aria-label=\"R\"><path d=\"m30.413 94.025q0.18759 0 0.2682-0.06976 0.08216-0.06976 0.08216-0.22944 0-0.15813-0.08216-0.22634-0.08062-0.06821-0.2682-0.06821h-0.25115v0.59376zm-0.25115 0.41238v0.87592h-0.59686v-2.3146h0.91157q0.45734 0 0.66973 0.15348 0.21394 0.15348 0.21394 0.48524 0 0.22944-0.11162 0.37672-0.11007 0.14728-0.33331 0.21704 0.12247 0.0279 0.21859 0.12712 0.09767 0.09767 0.19689 0.29766l0.32401 0.65732h-0.63562l-0.28215-0.57516q-0.08527-0.17363-0.17363-0.2372-0.08682-0.06356-0.23254-0.06356z\" fill=\"#5a5a5a\"/></g></g></svg>";

constexpr auto muteOff = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"translate(-.13229 -.13229)\" aria-label=\"M\"><path d=\"m22.247 87.294h0.75964l0.5271 1.2387 0.5302-1.2387h0.75809v2.3146h-0.56431v-1.6929l-0.5333 1.248h-0.37827l-0.5333-1.248v1.6929h-0.56586z\" fill=\"#c8c8c8\"/></g></g></svg>";
constexpr auto muteOn  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"translate(-.13229 -.13229)\" aria-label=\"M\"><path d=\"m22.247 87.294h0.75964l0.5271 1.2387 0.5302-1.2387h0.75809v2.3146h-0.56431v-1.6929l-0.5333 1.248h-0.37827l-0.5333-1.248v1.6929h-0.56586z\" fill=\"#c8c8c8\"/></g></g></svg>";

constexpr auto soloOff = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"translate(.93228 -.28173)\" aria-label=\"S\"><path d=\"m23.231 87.515v0.48989q-0.19069-0.08527-0.37207-0.12867-0.18138-0.04341-0.34262-0.04341-0.21394 0-0.31626 0.05891t-0.10232 0.18293q0 0.09302 0.06821 0.14573 0.06976 0.05116 0.25115 0.08837l0.25425 0.05116q0.38602 0.07751 0.5488 0.23564 0.16278 0.15813 0.16278 0.44958 0 0.38292-0.22789 0.57051-0.22634 0.18604-0.69298 0.18604-0.22014 0-0.44183-0.04186t-0.44338-0.12402v-0.50384q0.22169 0.11782 0.42788 0.17828 0.20774 0.05891 0.39998 0.05891 0.19534 0 0.29921-0.06511 0.10387-0.06511 0.10387-0.18604 0-0.10852-0.07131-0.16743-0.06976-0.05891-0.2806-0.10542l-0.23099-0.05116q-0.34727-0.07441-0.5085-0.2372-0.15968-0.16278-0.15968-0.43873 0-0.34572 0.22324-0.53175t0.64182-0.18604q0.19069 0 0.39222 0.02945 0.20154 0.02791 0.41703 0.08527z\" fill=\"#c8c8c8\"/></g></g></svg>";
constexpr auto soloOn  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"translate(.93228 -.28173)\" aria-label=\"S\"><path d=\"m23.231 87.515v0.48989q-0.19069-0.08527-0.37207-0.12867-0.18138-0.04341-0.34262-0.04341-0.21394 0-0.31626 0.05891t-0.10232 0.18293q0 0.09302 0.06821 0.14573 0.06976 0.05116 0.25115 0.08837l0.25425 0.05116q0.38602 0.07751 0.5488 0.23564 0.16278 0.15813 0.16278 0.44958 0 0.38292-0.22789 0.57051-0.22634 0.18604-0.69298 0.18604-0.22014 0-0.44183-0.04186t-0.44338-0.12402v-0.50384q0.22169 0.11782 0.42788 0.17828 0.20774 0.05891 0.39998 0.05891 0.19534 0 0.29921-0.06511 0.10387-0.06511 0.10387-0.18604 0-0.10852-0.07131-0.16743-0.06976-0.05891-0.2806-0.10542l-0.23099-0.05116q-0.34727-0.07441-0.5085-0.2372-0.15968-0.16278-0.15968-0.43873 0-0.34572 0.22324-0.53175t0.64182-0.18604q0.19069 0 0.39222 0.02945 0.20154 0.02791 0.41703 0.08527z\" fill=\"#c8c8c8\"/></g></g></svg>";

constexpr auto upOff   = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"rotate(-90)\" fill=\"#c8c8c8\" aria-label=\"&gt;\"><path d=\"m-89.313 22.889v-0.38757l1.9875 0.71779v0.36587l-1.9875 0.71778v-0.38757l1.496-0.5116z\"/></g></g></svg>";
constexpr auto upOn    = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"rotate(-90)\" fill=\"#c8c8c8\" aria-label=\"&gt;\"><path d=\"m-89.313 22.889v-0.38757l1.9875 0.71779v0.36587l-1.9875 0.71778v-0.38757l1.496-0.5116z\"/></g></g></svg>";
constexpr auto downOff = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"rotate(-90)\" fill=\"#c8c8c8\" aria-label=\"&gt;\"><path d=\"m-87.325 23.915v0.38757l-1.9875-0.71779v-0.36587l1.9875-0.71778v0.38757l-1.496 0.5116z\"/></g></g></svg>";
constexpr auto downOn  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"rotate(-90)\" fill=\"#c8c8c8\" aria-label=\"&gt;\"><path d=\"m-87.325 23.915v0.38757l-1.9875-0.71779v-0.36587l1.9875-0.71778v0.38757l-1.496 0.5116z\"/></g></g></svg>";

constexpr auto removeOff = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"scale(-1)\" fill=\"#c8c8c8\" aria-label=\"x\"><path d=\"m-23.721-88.339-0.62632-0.84801h0.58756l0.35502 0.5147 0.35967-0.5147h0.58756l-0.62632 0.84491 0.65732 0.89142h-0.58756l-0.39067-0.5488-0.38602 0.5488h-0.58756z\"/></g></g></svg>";
constexpr auto removeOn  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-21.021 -85.938)\"><rect x=\"21.021\" y=\"85.938\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"scale(-1)\" fill=\"#c8c8c8\" aria-label=\"x\"><path d=\"m-23.721-88.339-0.62632-0.84801h0.58756l0.35502 0.5147 0.35967-0.5147h0.58756l-0.62632 0.84491 0.65732 0.89142h-0.58756l-0.39067-0.5488-0.38602 0.5488h-0.58756z\"/></g></g></svg>";

constexpr auto plusOff  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"scale(-1)\" fill=\"#c8c8c8\" aria-label=\"+\"><path d=\"m-37.658-120.97v0.99288h0.98909v0.44718h-0.98909v0.99287h-0.45096v-0.99287h-0.98909v-0.44718h0.98909v-0.99288z\"/></g></g></svg>";
constexpr auto plusOn   = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"scale(-1)\" fill=\"#c8c8c8\" aria-label=\"+\"><path d=\"m-37.658-120.97v0.99288h0.98909v0.44718h-0.98909v0.99287h-0.45096v-0.99287h-0.98909v-0.44718h0.98909v-0.99288z\"/></g></g></svg>";
constexpr auto minusOff = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"rotate(180 .12913 -.018559)\" fill=\"#c8c8c8\" aria-label=\"-\"><path d=\"m-38.22-120.07h1.1899v0.55139h-1.1899z\"/></g></g></svg>";
constexpr auto minusOn  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"rotate(180 .12913 -.018559)\" fill=\"#c8c8c8\" aria-label=\"-\"><path d=\"m-38.22-120.07h1.1899v0.55139h-1.1899z\"/></g></g></svg>";

constexpr auto armOff      = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><path d=\"m39.206 119.76a1.3229 1.3229 0 0 1-1.3229 1.3229 1.3229 1.3229 0 0 1-1.3229-1.3229 1.3229 1.3229 0 0 1 1.3229-1.3229 1.3229 1.3229 0 0 1 1.3229 1.3229z\" fill=\"#b18e8e\" stroke-width=\"5.2917\"/></g></svg>";
constexpr auto armOn       = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><path d=\"m39.206 119.76a1.3229 1.3229 0 0 1-1.3229 1.3229 1.3229 1.3229 0 0 1-1.3229-1.3229 1.3229 1.3229 0 0 1 1.3229-1.3229 1.3229 1.3229 0 0 1 1.3229 1.3229z\" fill=\"#ae53d0\" stroke-width=\"5.2917\"/></g></svg>";
constexpr auto armDisabled = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><path d=\"m39.206 119.76a1.3229 1.3229 0 0 1-1.3229 1.3229 1.3229 1.3229 0 0 1-1.3229-1.3229 1.3229 1.3229 0 0 1 1.3229-1.3229 1.3229 1.3229 0 0 1 1.3229 1.3229z\" fill=\"#4e4e4e\" stroke-width=\"5.2917\"/></g></svg>";

constexpr auto channelPlayOff = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><path d=\"m36.719 118.39 2.3283 1.3696-2.3283 1.3696z\" color=\"#000000\" fill=\"#b18e8e\"/></g></svg>";
constexpr auto channelPlayOn  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><path d=\"m36.719 118.39 2.3283 1.3696-2.3283 1.3696z\" color=\"#000000\" fill=\"#ae53d0\"/></g></svg>";

constexpr auto divideOff   = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"matrix(.30533 0 0 .30533 -204.57 -55.909)\" fill=\"#c8c8c8\"><path d=\"m793.5 574.25v-1.1011h1.1011v1.1011zm3.1528 1.52h-5.1992v-0.90235h5.1992zm-3.1528 1.7188v-1.1011h1.1011v1.1011z\" fill=\"#c8c8c8\"/></g></g></svg>";
constexpr auto divideOn    = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"matrix(.30533 0 0 .30533 -204.57 -55.909)\" fill=\"#c8c8c8\"><path d=\"m793.5 574.25v-1.1011h1.1011v1.1011zm3.1528 1.52h-5.1992v-0.90235h5.1992zm-3.1528 1.7188v-1.1011h1.1011v1.1011z\" fill=\"#c8c8c8\"/></g></g></svg>";
constexpr auto multiplyOff = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"matrix(.33895 0 0 .33895 -225.22 -75.247)\" fill=\"#c8c8c8\"><path d=\"m773.9 577.01 1.6919-1.6919-1.6865-1.6865 0.65527-0.65527 1.6865 1.6865 1.6812-1.6812 0.64453 0.6499-1.6758 1.6812 1.6865 1.6865-0.65527 0.65527-1.6865-1.6865-1.6919 1.6919z\" fill=\"#c8c8c8\"/></g></g></svg>";
constexpr auto multiplyOn  = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-35.502 -117.38)\"><rect x=\"35.502\" y=\"117.38\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#4e4e4e\"/><g transform=\"matrix(.33895 0 0 .33895 -225.22 -75.247)\" fill=\"#c8c8c8\"><path d=\"m773.9 577.01 1.6919-1.6919-1.6865-1.6865 0.65527-0.65527 1.6865 1.6865 1.6812-1.6812 0.64453 0.6499-1.6758 1.6812 1.6865 1.6865-0.65527 0.65527-1.6865-1.6865-1.6919 1.6919z\" fill=\"#c8c8c8\"/></g></g></svg>";

constexpr auto loopBasic         = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><path d=\"m71.298 107.72c-0.87283 0-1.5875 0.71466-1.5875 1.5875 0 0.87283 0.71467 1.5875 1.5875 1.5875s1.5875-0.71467 1.5875-1.5875c0-0.87284-0.71467-1.5875-1.5875-1.5875zm0 0.66145c0.51536 0 0.92604 0.41069 0.92604 0.92605 0 0.51535-0.41069 0.92604-0.92604 0.92604-0.51536 0-0.92604-0.41069-0.92604-0.92604 0-0.51536 0.41069-0.92605 0.92604-0.92605z\" color=\"#000000\" fill=\"#c8c8c8\"/></g></svg>";
constexpr auto loopOnce          = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><path d=\"m71.143 107.73c-0.7239 0.0725-1.3151 0.62054-1.4325 1.3219h0.67024c0.11704-0.39537 0.4824-0.68575 0.92139-0.68575 0.43899 0 0.80435 0.29038 0.92139 0.68575h0.66197c-0.12679-0.75066-0.78568-1.3219-1.5834-1.3219-0.05552 0-0.10509-5e-3 -0.15916 0zm-1.4242 1.8505c0.13015 0.7467 0.78853 1.3219 1.5834 1.3219 0.79484 0 1.4532-0.57519 1.5834-1.3219h-0.66197c-0.11704 0.39537-0.4824 0.68575-0.92139 0.68575s-0.80435-0.29038-0.92139-0.68575z\" fill=\"#c8c8c8\"/></g></svg>";
constexpr auto loopOnceBar       = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"translate(3.9608 1.3499)\" fill=\"#c8c8c8\"><path d=\"m67.182 106.38c-0.7239 0.0725-1.3151 0.62054-1.4325 1.3219h0.67024c0.11704-0.39537 0.4824-0.68575 0.92139-0.68575 0.43899 0 0.80435 0.29038 0.92139 0.68575h0.66197c-0.12679-0.75065-0.78568-1.3219-1.5834-1.3219-0.05552 0-0.10509-5e-3 -0.15916 0zm-1.4242 1.8505c0.13015 0.74669 0.78853 1.3219 1.5834 1.3219 0.79484 0 1.4532-0.57519 1.5834-1.3219h-0.66197c-0.11704 0.39537-0.4824 0.68575-0.92139 0.68575s-0.80435-0.29038-0.92139-0.68575z\"/><path d=\"m67.866 107.96a0.52917 0.52917 0 0 1-0.52917 0.52917 0.52917 0.52917 0 0 1-0.52917-0.52917 0.52917 0.52917 0 0 1 0.52917-0.52916 0.52917 0.52917 0 0 1 0.52917 0.52916z\"/></g></g></svg>";
constexpr auto loopRepeat        = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"matrix(.26458 0 0 .26458 -54.114 90.64)\" fill=\"#c8c8c8\"><path d=\"m468 64.566v2.4062c1.9882 0 3.5938 1.607 3.5938 3.5938 0 1.9867-1.6055 3.5938-3.5938 3.5938v2.4062c3.3137 0 6-2.6888 6-6 0-3.3112-2.6863-6-6-6z\"/><path d=\"m480 64.566v2.4062c-1.9882 0-3.5938 1.607-3.5938 3.5938 0 1.9867 1.6055 3.5938 3.5938 3.5938v2.4062c-3.3137 0-6-2.6888-6-6 0-3.3112 2.6863-6 6-6z\"/></g></g></svg>";
constexpr auto oneshotBasic      = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><path d=\"m69.711 110.24v0.66146h3.175v-0.66146h-3.175z\" fill=\"#c8c8c8\"/></g></svg>";
constexpr auto oneshotBasicPause = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"matrix(.26458 0 0 .26458 -49.634 54.181)\"><path d=\"m460.57 202.37v9.5h-9.5v2.5h12v-12h-2.5z\" fill=\"#c8c8c8\"/></g></g></svg>";
constexpr auto oneshotPress      = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><g transform=\"matrix(.26458 0 0 .26458 -138.25 -86.841)\" fill=\"#c8c8c8\"><path d=\"m786 744.86v2.5h12v-2.5h-12z\" fill=\"#c8c8c8\"/><path transform=\"matrix(1.6667 0 0 1.6667 437.67 56.079)\" d=\"m212 410.86a1.5 1.5 0 0 1-1.5 1.5 1.5 1.5 0 0 1-1.5-1.5 1.5 1.5 0 0 1 1.5-1.5 1.5 1.5 0 0 1 1.5 1.5z\"/></g></g></svg>";
constexpr auto oneshotRetrig     = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><path d=\"m71.629 110.9c0.54745 9e-3 1.0673-0.3844 1.2091-0.91272 0.15205-0.50806-0.06202-1.0961-0.50639-1.3862-0.25347-0.17833-0.57038-0.23348-0.87477-0.21272-0.58205 5e-3 -1.1641-2e-3 -1.7462-2e-3v0.66146c0.65491 8e-4 1.3099-2e-3 1.9648 1e-3 0.33614 0.0132 0.61089 0.35217 0.55024 0.684-0.03987 0.30123-0.33589 0.53152-0.63691 0.50543h-1.8781v0.66146c0.63941-1.2e-4 1.2788 0 1.9182 0z\" fill=\"#c8c8c8\"/></g></svg>";
constexpr auto oneshotEndless    = "<svg width=\"18\" height=\"18\" version=\"1.1\" viewBox=\"0 0 4.7625 4.7625\"><g transform=\"translate(-68.917 -106.93)\"><rect x=\"68.917\" y=\"106.93\" width=\"4.7625\" height=\"4.7625\" color=\"#000000\" fill=\"#252525\"/><path d=\"m71.635 108.39c-0.54745-9e-3 -1.0673 0.3844-1.2091 0.91271-0.09175 0.31166-0.04893 0.65658 0.10943 0.93937h-0.81855v0.66146c0.67161-3e-3 1.3435 5e-3 2.0149-4e-3 0.55611-0.0331 1.051-0.48101 1.1404-1.0306 0.10098-0.5195-0.16915-1.0819-0.63805-1.3276-0.18285-0.0997-0.39088-0.15205-0.59909-0.15168zm-0.01654 0.66146c0.33696-0.0219 0.64615 0.28819 0.61935 0.62567-0.0085 0.32757-0.32505 0.59863-0.64933 0.56376-0.33614-0.0132-0.61089-0.35217-0.55024-0.68399 0.03884-0.28014 0.29715-0.50573 0.58022-0.50544z\" fill=\"#c8c8c8\"/></g></svg>";

#ifdef WITH_VST
constexpr auto vstLogo = "<svg width=\"100\" height=\"31.6\" version=\"1.1\" viewBox=\"0 0 26.458 8.3608\"><g transform=\"translate(-5.587 -141.92)\"><path class=\"st0\" d=\"m14.609 148.75v-1.5346l0.13229 0.0265c0.07937 0 0.23812 0.0265 0.39688 0.0794 0.23812 0.0265 0.26458 0.0265 0.37042 0.26458 0.23812 0.52917 0.66146 1.0319 1.1377 1.3758 0.89958 0.66146 2.3812 1.0054 3.8629 0.87313 0.97896-0.0794 1.5081-0.26458 1.8785-0.74083 0.23812-0.26459 0.29104-0.52917 0.15875-0.79375-0.21167-0.4498-0.74083-0.68792-2.9898-1.2965-0.76729-0.18521-1.614-0.42334-1.8521-0.50271-1.1112-0.34396-2.0638-0.79375-2.3548-1.1377l-0.13229-0.15875 0.10583-0.15875c0.47625-0.66146 0.66146-0.76729 1.3229-0.87312 0.21167-0.0265 0.39688-0.0794 0.39688-0.0794 0-0.0265-0.635 0.0265-0.9525 0.10583-0.44979 0.1323-0.47625 0.18521-2.831 3.7042l-1.5346 2.3019h-0.47625l-2.1167-2.831c-1.1642-1.561-2.196-2.9104-2.2754-2.9898-0.18521-0.15875-0.47625-0.23812-1.0319-0.3175-0.23812-0.0265-0.23812-0.0265-0.23812-0.18521v-0.15875h5.6092v0.15875c0 0.18521-0.02646 0.18521-0.76729 0.23813-0.39688 0.0265-0.60854 0.10583-0.60854 0.3175 0 0.0794 0.23813 0.42333 0.50271 0.79375 1.5875 2.2225 2.0108 2.831 2.0638 2.831 0.05292 0 1.8521-2.7252 2.1431-3.1485 0.15875-0.3175 0.23812-0.52917 0.10583-0.66146-0.07938-0.0794-0.47625-0.15875-0.89958-0.18521h-0.34396v-0.34396h0.87312c0.79375 0 0.87312 0 0.87312-0.10583 0.02646-0.15875 0.50271-0.60854 0.87312-0.84667 0.87312-0.60854 2.0638-0.84666 4.0217-0.76729 0.9525 0.0265 1.3229 0.0265 2.196 0.18521 1.2435 0.18521 1.4023 0.18521 1.6933-0.0794 0.15875-0.18521 0.21167-0.18521 0.47625-0.18521h0.29104v0.87312 1.2965 0.42333h-0.34396c-0.21167 0-0.34396-0.0265-0.34396-0.0265 0-0.0265-0.15875-0.26459-0.39688-0.52917-0.87312-1.0054-1.9579-1.4817-3.4925-1.561-1.6404-0.0794-2.7781 0.42333-2.7252 1.1377 0.07938 0.58208 0.47625 0.79375 2.6194 1.3758 1.9579 0.52917 2.2225 0.60854 2.249 0.58208 0 0 0.02646-0.44979 0.02646-1.0054v-0.9525h0.44979 0.29104l0.23812 0.23812 0.42333 0.47625-0.47625 0.42333c-0.23812 0.26459-0.37042 0.4498-0.47625 0.74084 0 0.0265-0.07937 0.15875-0.02646 0.18521 0 0.0794 0 0.0794 0.44979 0.23812 1.2435 0.42333 1.9579 1.0319 2.1167 1.7727 0.10583 0.50271-0.10583 1.0319-0.635 1.4552-1.0583 0.82021-2.3812 1.1642-4.7625 1.1906-1.27 0-1.7727-0.0265-2.9898-0.26458-0.89958-0.18521-1.27-0.10583-1.4552 0.18521-0.02646 0.10583-0.10583 0.10583-0.44979 0.10583h-0.39688z\" fill=\"#95999F\"/><path class=\"st0\" d=\"m24.24 150.04c0-0.0265-0.02646-0.15875 0-0.23813 0-0.15875 0.02646-0.15875 0.26458-0.15875 0.44979-0.0265 0.87312-0.10583 1.0848-0.23812 0.13229-0.0794 0.21167-0.15875 0.23812-0.3175 0.02646-0.10584 0.02646-1.1112 0.02646-2.6723v-2.4606h-1.1112v-0.37042h3.8365 3.466v2.0108h-0.26458c-0.13229 0-0.26458-0.0265-0.26458-0.0794 0-0.15875-0.21167-0.52917-0.39688-0.76729-0.39688-0.44979-1.0848-0.68792-2.1431-0.76729l-0.58208-0.0265v2.5929c0 1.9579 0 2.5929 0.07938 2.7252 0.10583 0.23813 0.37042 0.34396 1.0054 0.34396l0.58208 0.0265v0.47625h-2.9369c-2.4077-0.0529-2.8575-0.0529-2.884-0.0794z\" fill=\"#95999F\"/></g></svg>";
#endif
} // namespace giada::v::graphics

#endif