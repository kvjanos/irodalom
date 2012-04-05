/************************************************************
 * xfig_object.h                                            *
 *----------------------------------------------------------*
 *  Header file defining constants and macros for xfigure   *
 *   creation.                                              *
 *----------------------------------------------------------*
 *  Authors: Habib Moukalled, Kenton Oliver, Song Wang      *  
 *  Last modified: September 06, 2006                       *
 ************************************************************/


#include <string.h>

#ifndef XFIG_OBJECT_H
#define XFIG_OBJECT_H

#define XFIG FILE

#define LANDSCAPE "Landscape"
#define PORTRAIT "Portrait"
#define CENTER "Center"
#define FLUSH "Flush Left"
#define METRIC "Metric"
#define INCHES "Inches"
#define LETTER "Letter"
#define LEGAL "Legal"
#define LEDGER "Ledger"
#define TABLOID "Tabloid"
#define A "A"
#define B "B"
#define C "C"
#define D "D"
#define E "E"
#define A4 "A4"
#define A3 "A3"
#define A2 "A2"
#define A1 "A1"
#define A0 "A0"
#define B5 "B5"
#define SINGLE "Single"
#define MULTIPLE "Multiple"

#define PSEUDO_COLOR_TYPE 0
#define ELLIPSE_TYPE 1
#define POLYLINE_TYPE 2
#define SPLINE_TYPE 3
#define TEXT_TYPE 4
#define ARC_TYPE 5
#define COMPOUND_TYPE 6

#define RIGID_TEXT 1
#define LATEX_TEXT 2
#define POSTSCRIPT_TEXT 4
#define HIDDEN_TEXT 8

/**************************************************************************
 * The two color fields (pen and fill; pen only for text) are defined as: *
 *  USER0 - USER511 (32 - 543) are user colors defined in color pseudo    *
 *  objects (type 0)                                                      *
 **************************************************************************/
typedef enum COLOR {BACKGROUND = -3, NONE, DEFAULT_COLOR, BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE, BLUE0, BLUE1, BLUE2, BLUE3, GREEN0, GREEN1, GREEN2, CYAN0, CYAN1, CYAN2, RED0, RED1, RED2, MAGENTA0, MAGENTA1, MAGENTA2, BROWN0, BROWN1, BROWN2, PINK0, PINK1, PINK2, PINK3, GOLD, USER0, USER1, USER2, USER3, USER4, USER5, USER6, USER7, USER8, USER9, USER10, USER11, USER12, USER13, USER14, USER15, USER16, USER17, USER18, USER19, USER20, USER21, USER22, USER23, USER24, USER25, USER26, USER27, USER28, USER29, USER30, USER31, USER32, USER33, USER34, USER35, USER36, USER37, USER38, USER39, USER40, USER41, USER42, USER43, USER44, USER45, USER46, USER47, USER48, USER49, USER50, USER51, USER52, USER53, USER54, USER55, USER56, USER57, USER58, USER59, USER60, USER61, USER62, USER63, USER64, USER65, USER66, USER67, USER68, USER69, USER70, USER71, USER72, USER73, USER74, USER75, USER76, USER77, USER78, USER79, USER80, USER81, USER82, USER83, USER84, USER85, USER86, USER87, USER88, USER89, USER90, USER91, USER92, USER93, USER94, USER95, USER96, USER97, USER98, USER99, USER100, USER101, USER102, USER103, USER104, USER105, USER106, USER107, USER108, USER109, USER110, USER111, USER112, USER113, USER114, USER115, USER116, USER117, USER118, USER119, USER120, USER121, USER122, USER123, USER124, USER125, USER126, USER127, USER128, USER129, USER130, USER131, USER132, USER133, USER134, USER135, USER136, USER137, USER138, USER139, USER140, USER141, USER142, USER143, USER144, USER145, USER146, USER147, USER148, USER149, USER150, USER151, USER152, USER153, USER154, USER155, USER156, USER157, USER158, USER159, USER160, USER161, USER162, USER163, USER164, USER165, USER166, USER167, USER168, USER169, USER170, USER171, USER172, USER173, USER174, USER175, USER176, USER177, USER178, USER179, USER180, USER181, USER182, USER183, USER184, USER185, USER186, USER187, USER188, USER189, USER190, USER191, USER192, USER193, USER194, USER195, USER196, USER197, USER198, USER199, USER200, USER201, USER202, USER203, USER204, USER205, USER206, USER207, USER208, USER209, USER210, USER211, USER212, USER213, USER214, USER215, USER216, USER217, USER218, USER219, USER220, USER221, USER222, USER223, USER224, USER225, USER226, USER227, USER228, USER229, USER230, USER231, USER232, USER233, USER234, USER235, USER236, USER237, USER238, USER239, USER240, USER241, USER242, USER243, USER244, USER245, USER246, USER247, USER248, USER249, USER250, USER251, USER252, USER253, USER254, USER255, USER256, USER257, USER258, USER259, USER260, USER261, USER262, USER263, USER264, USER265, USER266, USER267, USER268, USER269, USER270, USER271, USER272, USER273, USER274, USER275, USER276, USER277, USER278, USER279, USER280, USER281, USER282, USER283, USER284, USER285, USER286, USER287, USER288, USER289, USER290, USER291, USER292, USER293, USER294, USER295, USER296, USER297, USER298, USER299, USER300, USER301, USER302, USER303, USER304, USER305, USER306, USER307, USER308, USER309, USER310, USER311, USER312, USER313, USER314, USER315, USER316, USER317, USER318, USER319, USER320, USER321, USER322, USER323, USER324, USER325, USER326, USER327, USER328, USER329, USER330, USER331, USER332, USER333, USER334, USER335, USER336, USER337, USER338, USER339, USER340, USER341, USER342, USER343, USER344, USER345, USER346, USER347, USER348, USER349, USER350, USER351, USER352, USER353, USER354, USER355, USER356, USER357, USER358, USER359, USER360, USER361, USER362, USER363, USER364, USER365, USER366, USER367, USER368, USER369, USER370, USER371, USER372, USER373, USER374, USER375, USER376, USER377, USER378, USER379, USER380, USER381, USER382, USER383, USER384, USER385, USER386, USER387, USER388, USER389, USER390, USER391, USER392, USER393, USER394, USER395, USER396, USER397, USER398, USER399, USER400, USER401, USER402, USER403, USER404, USER405, USER406, USER407, USER408, USER409, USER410, USER411, USER412, USER413, USER414, USER415, USER416, USER417, USER418, USER419, USER420, USER421, USER422, USER423, USER424, USER425, USER426, USER427, USER428, USER429, USER430, USER431, USER432, USER433, USER434, USER435, USER436, USER437, USER438, USER439, USER440, USER441, USER442, USER443, USER444, USER445, USER446, USER447, USER448, USER449, USER450, USER451, USER452, USER453, USER454, USER455, USER456, USER457, USER458, USER459, USER460, USER461, USER462, USER463, USER464, USER465, USER466, USER467, USER468, USER469, USER470, USER471, USER472, USER473, USER474, USER475, USER476, USER477, USER478, USER479, USER480, USER481, USER482, USER483, USER484, USER485, USER486, USER487, USER488, USER489, USER490, USER491, USER492, USER493, USER494, USER495, USER496, USER497, USER498, USER499, USER500, USER501, USER502, USER503, USER504, USER505, USER506, USER507, USER508, USER509, USER510, USER511} COLOR;

/****************************************************
 * Area fill is defined for white, black, and other *
 ****************************************************/
typedef enum AREA_FILL {NO_FILL = -1, WHITE_BLACK = 0, BLACK_WHITE = 0, BLACK_AREA = 0, GREY0, GREY1, GREY2, GREY3, GREY4, GREY5, GREY6, GREY7, GREY8, GREY9, GREY10, GREY11, GREY12, GREY13, GREY14, GREY15, GREY16, GREY17, GREY18, WHITE_WHITE = 20, BLACK_BLACK = 20, SHADE0 = 1, SHADE1, SHADE2, SHADE3, SHADE4, SHADE5, SHADE6, SHADE7, SHADE8, SHADE9, SHADE10, SHADE11, SHADE12, SHADE13, SHADE14, SHADE15, SHADE16, SHADE17, SHADE18, FULL, TINT0, TINT1, TINT2, TINT3, TINT4, TINT5, TINT6, TINT7, TINT8, TINT9, TINT10, TINT11, TINT12, TINT13, TINT14, TINT15, TINT16, TINT17, TINT18, WHITE_AREA, LEFT_DIAG_30, RIGHT_DIAG_30, CROSSHATCH_30, LEFT_DIAG_45, RIGHT_DIAG_45, CROSSHATCH_45, HORIZONTAL_BRICKS, VERTICAL_BRICKS, HORIZONTAL_LINES, VERTICAL_LINES, CROSSHATCH, HORIZONTAL_SHINGLES_RIGHT, HORIZONTAL_SHINGLES_LEFT, VERTICAL_SHINGLES_0, VERTICAL_SHINGLES_1, FISH, SMALL_FISH, CIRCLES, HEXAGONS, OCTAGONS, HORIZONTAL_TIRE_TREADS, VERTICAL_TIRE_TREADS} AREA_FILL;

/************************
 * The line style field *
 ************************/
typedef enum LINE_STYLE {DEFAULT_LINE = -1, SOLID, DASHED, DOTTED, DASH_DOTTED, DASH_DOUBLE_DOTTED, DASH_TRIPPLE_DOTTED} LINE_STYLE;

/*****************************
 * The join style field:     *
 *  LINES only               *
 *****************************/
typedef enum JOIN_STYLE {MITER, ROUND, BEVEL} JOIN_STYLE;

/*****************************
 * The cap style field:      *
 *  LINES                    *
 *  OPEN_SPLINES             *
 *  ARCS                     *
 *****************************/
typedef enum CAP_STYLE {BUTT_CAP, ROUND_CAP, PROJECTING_CAP} CAP_STYLE;

/*****************************
 * The arrow type field:     *
 *  LINES                    *
 *  OPEN_SPLINES             *
 *  ARCS                     *
 *****************************/
typedef enum ARROW_TYPE {STICK, CLOSED_TRIANGLE, CLOSED_INDENTED, CLOSED_POINTED} ARROW_TYPE;

/*****************************
 * The arrow style field:    *
 *  LINES                    *
 *  OPEN_SPLINES             *
 *  ARCS                     *
 *****************************/
typedef enum ARROW_STYLE {HOLLOW, PEN_FILLED} ARROW_STYLE;

/****************
 * Arrow
 ****************/
typedef struct ARROW {
  ARROW_TYPE type;
  ARROW_STYLE style;
  float thickness, width, height;
} ARROW;


/*******************************************************
 * Font is defined in terms of bit 2 of the font_flags *
 * field.                                              *
 *  Bit 2 = 0 (LaTeX)                                  *
 *  Bit 2 = 1 (PostScript)                             *
 *******************************************************/
typedef enum FONT {LATEX_DEFAULT = 0, LATEX_ROMAN, LATEX_BOLD, LATEX_ITALIC, LATEX_SANS_SERIF, LATEX_TYPEWRITER, PS_DEFAULT = -1, TIMES_ROMAN, TIMES_ITALIC, TIMES_BOLD, TIMES_BOLD_ITALIC, AVANTGARDE_BOOK, AVANTGARDE_BOOK_OBLIQUE, AVANTGARDE_DEMI, AVANTGARDE_DEMI_OBLIQUE, BOOKMAN_LIGHT, BOOKMAN_LIGHT_ITALIC, BOOKMAN_DEMI, BOOKMAN_DEMI_ITALIC, COURIER, COURIER_OBLIQUE, COURIER_BOLD, COURIER_BOLD_OBLIQUE, HELVETICA, HELVETICA_OBLIQUE, HELVETICA_BOLD, HELVETICA_BOLD_OBLIQUE, HELVETICA_NARROW, HELVETICA_NARROW_OBLIQUE, HELVETICA_NARROW_BOLD, HELVETICA_NARROW_BOLD_OBLIQUE, NEW_CENTURY_SCHOOLBOOK_ROMAN, NEW_CENTURY_SCHOOLBOOK_ITALIC, NEW_CENTURY_SCHOOLBOOK_BOLD, NEW_CENTURY_SCHOOLBOOK_BOLD_ITALIC, PALATINO_ROMAN, PALATINO_ITALIC, PALATINO_BOLD, PALATINO_BOLD_ITALIC, SYMBOL, ZAPF_CHANCERY_MEDIUM_ITALIC, ZAPF_DINGBATS} FONT;

/****************
 * Subtype enum *
 ****************/
typedef enum SUB_TYPE {OPEN_ARC = 1, PIE_ARC, ELLIPSE_RADII = 1, ELLIPSE_DIAMETER, CIRCLE_RADIUS, CIRCLE_DIAMETER, SUB_POLYLINE = 1, BOX_POLYLINE, POLYGON_POLYLINE, ARC_BOX_POLYLINE, PICTURE_BOX_POLYLINE, OPEN_APPROX_SPLINE = 0, CLOSED_APPROX_SPLINE, OPEN_INTERPOLATED_SPLINE, CLOSED_INTERPOLATED_SPLINE, OPEN_XPLINE, CLOSED_XSPLINE, LEFT_TEXT = 0, CENTER_TEXT, RIGHT_TEXT} SUB_TYPE;

/************************************
 * Macros for creating xfig objects *
 ************************************/
#define XFIG_COMMENT(xfig, string) fprintf(xfig, "#%s\n", string)

#define XFIG_PSEUDO_COLOR(xfig, number, rgb) fprintf(xfig, "0 %d %s\n", number, rgb)

#define XFIG_ELLIPSE(xfig, sub_type, line_style, thickness, pen_color, fill_color, depth, area_fill, style_val, angle, center_x, center_y, radius_x, radius_y, start_x, start_y, end_x, end_y) \
  fprintf(xfig, "1 %d %d %d %d %d %d -1 %d %.2lf 1 %.2lf %d %d %d %d %d %d %d %d\n", sub_type, line_style, thickness, pen_color, fill_color, depth, area_fill, style_val, angle, \
    center_x, center_y, radius_x, radius_y, start_x, start_y, end_x, end_y)

//i is an indexing variable initialized by the user
#define XFIG_POLYLINE(xfig, i, sub_type, line_style, thickness, pen_color, fill_color, depth, area_fill, style_val, join_style, cap_style, radius, forward, backward, npoints, points, flipped, file, forward_type, forward_style, forward_thickness, forward_width, forward_height, backward_type, backward_style, backward_thickness, backward_width, backward_height) \
  fprintf(xfig, "2 %d %d %d %d %d %d -1 %d %.2lf %d %d %d %d %d %d\n", sub_type, line_style, thickness, pen_color, fill_color, depth, area_fill, style_val, join_style, cap_style, \
    radius, forward, backward, npoints); \
  if (forward == 1) { fprintf(xfig, "\t%d %d %d %d %d\n", forward_type, forward_style, forward_thickness, forward_width, forward_height); } \
  if (backward == 1) { fprintf(xfig, "\t%d %d %d %d %d\n", backward_type, backward_style, backward_thickness, backward_width, backward_height); } \
  if (sub_type == 5) { fprintf(xfig, "\t%d %s\n", flipped, file); } \
  fprintf(xfig, "\t"); \
  for (i = 0; i < npoints; ++i) { fprintf(xfig, "%d %d ", points[i*2], points[(i*2)+1]); } \
  fprintf(xfig, "\n")

//i is an indexing variable initialized by the user
#define XFIG_SPLINE(xfig, i, sub_type, line_style, thickness, pen_color, fill_color, depth, area_fill, style_val, cap_style, forward, backward, npoints, points, control, forward_type, forward_style, forward_thickness, forward_width, forward_height, backward_type, backward_style, backward_thickness, backward_width, backward_height) \
  fprintf(xfig, "3 %d %d %d %d %d %d -1 %d %.2lf %d %d %d %d\n", sub_type, line_style, thickness, pen_color, fill_color, depth, area_fill, style_val, cap_style, forward, backward, npoints); \
  if (forward == 1) { fprintf(xfig, "\t%d %d %d %d %d\n", forward_type, forward_style, forward_thickness, forward_width, forward_height); } \
  if (backward == 1) { fprintf(xfig, "\t%d %d %d %d %d\n", backward_type, backward_style, backward_thickness, backward_width, backward_height); } \
  fprintf(xfig, "\t"); \
  for (i = 0; i < npoints; ++i) { fprintf(xfig, "%d %d ", points[i*2], points[(i*2)+1]); } \
  fprintf(xfig, "\n\t"); \
  for (i = 0; i < npoints; ++i) { fprintf(xfig, "%d ", control[i]); } \
  fprintf(xfig, "\n")

#define XFIG_TEXT(xfig, sub_type, color, depth, font, font_size, angle, font_flags, height, length, x, y, string) \
  fprintf(xfig, "4 %d %d %d -1 %d %.2lf %.2lf %d %.2lf %.2lf %d %d %s\\001\n", sub_type, color, depth, font, font_size, angle, font_flags, height, length, x, y, string)

#define XFIG_ARC(xfig, sub_type, line_style, thickness, pen_color, fill_color, depth, area_fill, style_val, cap_style, direction, forward, backward, center_x, center_y, x1, y1, x2, y2, x3, y3, forward_type, forward_style, forward_thickness, forward_width, forward_height, backward_type, backward_style, backward_thickness, backward_width, backward_height) \
  fprintf(xfig, "5 %d %d %d %d %d %d -1 %d %,2lf %d %d %d %d %.2lf %.2lf, %d, %d, %d, %d, %d, %d\n", sub_type, line_style, thickness, pen_color, fill_color, depth, area_fill, style_val, cap_style, \
    direction, forward, backward, center_x, center_y, x1, y1, x2, y2, x3, y3); \
  if (forward == 1) { fprintf(xfig, "\t%d %d %d %d %d\n", forward_type, forward_style, forward_thickness, forward_width, forward_height); } \
  if (backward == 1) { fprintf(xfig, "\t%d %d %d %d %d", backward_type, backward_style, backward_thickness, backward_width, backward_height); } \
  fprintf(xfig, "\n")

#define XFIG_START_COMPOUND(xfig, upperleft_x, upperleft_y, lowerright_x, lowerright_y) fprintf(xfig, "6 %d %d %d %d\n", upperleft_x, upperleft_y, lowerright_x, lowerright_y)
#define XFIG_END_COMPOUND(xfig) fprintf(xfig, "-6\n")

#endif
