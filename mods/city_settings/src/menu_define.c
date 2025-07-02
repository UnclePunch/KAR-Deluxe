#include "menu.h"           // needed for macro def
#include "stadium_toggle.h" // needed for function that creates stadium toggle menu
#include "machine_toggle.h" // needed for function that creates stadium toggle menu
#include "event_toggle.h"   // needed for function that creates event toggle menu
#include "item_toggle.h"    // needed for function that creates item toggle menu

// Item Select Menu
DEFINE_MENU_CUSTOM(item_select, "Item Select", ItemToggle_Create);
// Stadium Select Menu
DEFINE_MENU_CUSTOM(stadium_select, "Stadium Select", StadiumToggle_Create);
// Event Select Menu
DEFINE_MENU_CUSTOM(event_select, "Event Select", EventToggle_Create);
// Machine Select Menu
DEFINE_MENU_CUSTOM(machine_select, "Machine Select", MachineToggle_Create);

// Item Settings Menu
DEFINE_MENU_GENERIC(item_settings, "Item Settings", 5,
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_FREQUENCY,
                                  DEFINE_SELECTION_VALUE(5, 2, CITYSETTING_SAVE_ITEMFREQ,
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_NONE, "No items will drop."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_LOW, "Few items will drop."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_ORIGINAL, "A moderate amount of items will drop."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_HIGH, "Many items will drop."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_VERY_HIGH, "An overwhelming amount of items will drop."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_DRAGOON,
                                  DEFINE_SELECTION_VALUE(3, 1, CITYSETTING_SAVE_DRAGOONSPAWN,
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_NEVER, "The Dragoon pieces will never spawn."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "The Dragoon pieces may spawn."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ALWAYS, "The Dragoon pieces will always spawn."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_HYDRA,
                                  DEFINE_SELECTION_VALUE(3, 1, CITYSETTING_SAVE_HYDRASPAWN,
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_NEVER, "The Hydra pieces will never spawn."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "The Hydra pieces may spawn."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ALWAYS, "The Hydra pieces will always spawn."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_ALLUP,
                                  DEFINE_SELECTION_VALUE(2, 0, CITYSETTING_SAVE_ALLUP,
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "All Ups will only drop before versing King Dedede."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ALWAYS, "All Ups will always have a chance to drop."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_RANDOM_ITEMS,
                                  DEFINE_SELECTION_MENU(&item_select, "Toggle which items may appear.")), );

// Stadium Settings Menu
DEFINE_MENU_GENERIC(stadium_settings, "Stadium Settings", 3,
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_STADIUM,
                                  DEFINE_SELECTION_VALUE(9, 0, CITYSETTING_SAVE_STADIUM,
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_SHUFFLE, "Which stadium will appear? It's a secret!"),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_DRAG_RACE, "Use only the stadium selected here!"),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_AIR_GLIDER, "Use only the stadium selected here!"),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_TARGET_FLIGHT, "Use only the stadium selected here!"),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_HIGH_JUMP, "Use only the stadium selected here!"),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_KIRBY_MELEE, "Use only the stadium selected here!"),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_DESTRUCTION_DERBY, "Use only the stadium selected here!"),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_SINGLE_RACE, "Use only the stadium selected here!"),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_VS_KING_DEDEDE, "Use only the stadium selected here!"), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_REVEAL,
                                  DEFINE_SELECTION_VALUE(2, 0, CITYSETTING_SAVE_STADIUMREVEAL,
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_OFF, "The stadium will not always be revealed."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ON, "Always accurately predict the stadium upon entering the city."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_RANDOM_STADIUM,
                                  DEFINE_SELECTION_MENU(&stadium_select, "Toggle which stadium events may appear.")), );

// Event Settings Menu
DEFINE_MENU_GENERIC(event_settings, "Event Settings", 3,
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_FREQUENCY,
                                  DEFINE_SELECTION_VALUE(5, 2, CITYSETTING_SAVE_EVENTFREQ,
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_NONE, "Events will not occur."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_LOW, "Events will rarely occur."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_ORIGINAL, "Events will regularly occur."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_HIGH, "Events will occur very often."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_VERY_HIGH, "Events will occur nonstop."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_ALLOW_REPEATS,
                                  DEFINE_SELECTION_VALUE(2, 0, CITYSETTING_SAVE_EVENTREPEAT,
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "The same event will not occur more than once."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ON, "The same event may occur more than once."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_RANDOM_EVENT,
                                  DEFINE_SELECTION_MENU(&event_select, "Toggle which events appear in the city.")), );

// Vehicle Settings Menu
DEFINE_MENU_GENERIC(machine_settings, "Machine Settings", 4,
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_STARTING_MACHINE,
                                  DEFINE_SELECTION_VALUE(20, 0, CITYSETTING_SAVE_MACHINESTART,
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_COMPACT_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_WARP_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_FLIGHT_WARP_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_WAGON_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_SWERVE_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_WINGED_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_SHADOW_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_BULK_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_ROCKET_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_JET_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_TURBO_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_SLICK_STAR, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_FORMULA, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_HYDRA, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_DRAGOON, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_WHEELIE_BIKE, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_WHEELIE_SCOOTER, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_REX_WHEELIE, "Start with this machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_RANDOM_SINGLE, "All players start with the same random machine."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_RANDOM_ALL, "All players start with a different random machine."), )),
                    // DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_MACHINE_PACK,
                    //               DEFINE_SELECTION_VALUE(3, 0, CITYSETTING_SAVE_MACHINEPACK,
                    //                                      DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "Play with the original machines from Kirby Air Ride."),
                    //                                      DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "Slick pack description here."),
                    //                                      DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "Play with machines that are more balanced."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_SPEED_LIMIT,
                                  DEFINE_SELECTION_VALUE(2, 0, CITYSETTING_SAVE_MACHINESPEED,
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "Machines will have a top speed limit."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_OFF, "Machine speeds will not be limited."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_MACHINE_RESPAWN,
                                  DEFINE_SELECTION_VALUE(2, 0, CITYSETTING_SAVE_MACHINERESPAWN,
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ORIGINAL, "Players will roam the City after their machine is destroyed."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_ON, "Players will respawn on their starting machine."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_RANDOM_MACHINE,
                                  DEFINE_SELECTION_MENU(&machine_select, "Toggle which machines will spawn in the city.")), );

// Additional Settings Menu
DEFINE_MENU_GENERIC(additional_settings, "Additional Settings", 2,
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_TEMPO,
                                  DEFINE_SELECTION_VALUE(2, 0, CITYSETTING_SAVE_TEMPO,
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_DEFAULT, "Play at normal game speed."),
                                                         DEFINE_VALUE(CITYSETTING_VALSMALLTEX_SLOW, "Play at a slower speed that's useful for practice."), )),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_VALUE, CITYSETTING_OPTTEX_DAMAGE_RATIO,
                                  DEFINE_SELECTION_VALUE(10, 2, CITYSETTING_SAVE_DAMAGE,
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_05, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_075, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_100, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_125, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_150, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_175, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_200, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_250, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_300, "All damage will be multiplied by this number."),
                                                         DEFINE_VALUE(CITYSETTING_VALLARGETEX_500, "All damage will be multiplied by this number."), )), );

// Top Menu
DEFINE_MENU_GENERIC(top_menu, "Game Settings", 6,
                    DEFINE_OPTION(CITYSETTING_OPTKIND_NUM, CITYSETTING_OPTTEX_TIME,
                                  DEFINE_SELECTION_NUM(1, 99, 7, CITYSETTING_NUMTEX_MIN, "Set the city time limit prior to the stadium.", CITYSETTING_SAVE_TIME)),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_ITEM_SETTINGS,
                                  DEFINE_SELECTION_MENU_GENERIC(&item_settings, "Make modifications to the items that drop.")),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_STADIUM_SETTINGS,
                                  DEFINE_SELECTION_MENU_GENERIC(&stadium_settings, "Adjust settings for the stadium that occurs after the city.")),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_EVENT_SETTINGS,
                                  DEFINE_SELECTION_MENU_GENERIC(&event_settings, "Change settings for the events that occur within the city.")),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_MACHINE_SETTINGS,
                                  DEFINE_SELECTION_MENU_GENERIC(&machine_settings, "Adjust machine settings.")),
                    DEFINE_OPTION(CITYSETTING_OPTKIND_MENU, CITYSETTING_MENUTEX_ADDITIONAL_SETTINGS,
                                  DEFINE_SELECTION_MENU_GENERIC(&additional_settings, "Set even more detailed rules here.")), );
