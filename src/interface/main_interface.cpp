#include "main_interface.hpp"

#include <algorithm>
#include <iostream>
#include <string>

Main_Interface::Main_Interface(int* argc, char*** argv, Board** board_ptr) : m_board_ptr(board_ptr)
{
    // Initialize GTK
    gtk_init(argc, argv);

    // Initialize starting screen
    Initialize_Window();
    Initialize_Input_Screen();

    // Show all elements
    gtk_widget_show_all(m_window_ptr);

    // Start loop
    gtk_main();
}

void Main_Interface::Initialize_Window()
{
    // Create window
    m_window_ptr = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // Set window properties
    gtk_window_set_title(GTK_WINDOW(m_window_ptr), "Boggle Solver");
    gtk_window_set_default_size(GTK_WINDOW(m_window_ptr), 750, 750);
    gtk_widget_set_size_request(m_window_ptr, 600, 500);
    gtk_window_set_position(GTK_WINDOW(m_window_ptr), GTK_WIN_POS_CENTER);

    // Setup close button
    g_signal_connect(m_window_ptr, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}

void Main_Interface::Initialize_Input_Screen()
{
    // Create widgets
    m_input_screen_frame_ptr = gtk_aspect_frame_new("", 0.5, 0.5, (double)(*m_board_ptr)->Get_Board_Size() / (double)((*m_board_ptr)->Get_Board_Size() + 1.5), gtk_false());
    m_input_grid_ptr = gtk_grid_new();
    m_board_size_4x4_button_ptr = gtk_toggle_button_new_with_label("4x4");
    m_board_size_5x5_button_ptr = gtk_toggle_button_new_with_label("5x5");
    m_randomize_button_ptr = gtk_button_new_with_label("Randomize!");
    m_solve_button_ptr = gtk_button_new_with_label("Solve!");

    // Disable solve button
    gtk_widget_set_sensitive(m_solve_button_ptr, gtk_false());

    // Set frame properties
    gtk_frame_set_shadow_type(GTK_FRAME(m_input_screen_frame_ptr), GTK_SHADOW_NONE);
    gtk_container_set_border_width(GTK_CONTAINER(m_input_screen_frame_ptr), 10);

    // Set input grid properties
    gtk_grid_set_row_spacing(GTK_GRID(m_input_grid_ptr), 5.0);
    gtk_grid_set_column_spacing(GTK_GRID(m_input_grid_ptr), 5.0);
    gtk_grid_set_row_homogeneous(GTK_GRID(m_input_grid_ptr), gtk_true());
    gtk_grid_set_column_homogeneous(GTK_GRID(m_input_grid_ptr), gtk_true());

    // Create font attributes
    PangoAttrList* font_attribute_list = pango_attr_list_new();
    PangoAttribute* font_size_attribute = pango_attr_size_new_absolute(PANGO_SCALE * 40.0);
    pango_attr_list_insert(font_attribute_list, font_size_attribute);

    // Add text input boxes to grid
    m_entry_box_ptrs.resize((*m_board_ptr)->Get_Board_Size());
    for (uint8_t x = 0; x < (*m_board_ptr)->Get_Board_Size(); x++)
    {
        m_entry_box_ptrs[x].resize((*m_board_ptr)->Get_Board_Size());
        for (uint8_t y = 0; y < (*m_board_ptr)->Get_Board_Size(); y++)
        {
            // Create entry box
            m_entry_box_ptrs[x][y] = gtk_entry_new();

            gtk_entry_set_alignment(GTK_ENTRY(m_entry_box_ptrs[x][y]), 0.5);                  // Center text in entry box
            gtk_entry_set_attributes(GTK_ENTRY(m_entry_box_ptrs[x][y]), font_attribute_list); // Apply font attributes to entry box

            // Allocate structure to pass to callback
            Input_Box_Callback_Data* input_box_callback_data_ptr = (Input_Box_Callback_Data*)malloc(sizeof(Input_Box_Callback_Data));
            input_box_callback_data_ptr->interface = this;
            input_box_callback_data_ptr->x_position = x;
            input_box_callback_data_ptr->y_position = y;

            // Add entry box to grid
            gtk_grid_attach(GTK_GRID(m_input_grid_ptr), m_entry_box_ptrs[x][y], x * 2, y * 2, 2, 2);

            // Setup changed and destroyed callbacks
            g_signal_connect(GTK_EDITABLE(m_entry_box_ptrs[x][y]), "changed", G_CALLBACK(On_Input_Changed), (gpointer)input_box_callback_data_ptr);
            g_signal_connect(m_entry_box_ptrs[x][y], "destroy", G_CALLBACK(On_Input_Box_Destroy), (gpointer)input_box_callback_data_ptr);
        }
    }

    // Activate size button
    m_is_4x4 = (*m_board_ptr)->Get_Board_Size() == 4;
    if (m_is_4x4)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_board_size_4x4_button_ptr), gtk_true());
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_board_size_5x5_button_ptr), gtk_false());
    }
    else
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_board_size_5x5_button_ptr), gtk_true());
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_board_size_4x4_button_ptr), gtk_false());
    }

    // Add buttons to grid
    gtk_grid_attach(GTK_GRID(m_input_grid_ptr), m_board_size_4x4_button_ptr, 0, (*m_board_ptr)->Get_Board_Size() * 2, (*m_board_ptr)->Get_Board_Size(), 1);
    gtk_grid_attach(GTK_GRID(m_input_grid_ptr), m_board_size_5x5_button_ptr, (*m_board_ptr)->Get_Board_Size(), (*m_board_ptr)->Get_Board_Size() * 2, (*m_board_ptr)->Get_Board_Size(), 1);
    gtk_grid_attach(GTK_GRID(m_input_grid_ptr), m_randomize_button_ptr, 0, (*m_board_ptr)->Get_Board_Size() * 2 + 1, (*m_board_ptr)->Get_Board_Size() * 2, 1);
    gtk_grid_attach(GTK_GRID(m_input_grid_ptr), m_solve_button_ptr, 0, (*m_board_ptr)->Get_Board_Size() * 2 + 2, (*m_board_ptr)->Get_Board_Size() * 2, 1);

    // Setup button callbacks
    g_signal_connect(m_board_size_4x4_button_ptr, "clicked", G_CALLBACK(On_4x4_Clicked), this);
    g_signal_connect(m_board_size_5x5_button_ptr, "clicked", G_CALLBACK(On_5x5_Clicked), this);
    g_signal_connect(m_randomize_button_ptr, "clicked", G_CALLBACK(On_Randomized_Clicked), this);
    g_signal_connect(m_solve_button_ptr, "clicked", G_CALLBACK(On_Solve_Clicked), this);

    // Add grid to frame and the frame to the window
    gtk_container_add(GTK_CONTAINER(m_input_screen_frame_ptr), m_input_grid_ptr);
    gtk_container_add(GTK_CONTAINER(m_window_ptr), m_input_screen_frame_ptr);

    // Show all elements
    gtk_widget_show_all(m_window_ptr);
}

void Main_Interface::Initialize_Solution_Screen()
{
    // Get vector of solutions
    m_solutions = m_solver_ptr->Get_Solutions();

    // Replace 1, 2, 3, 4, 5, 6 with QU, IN, TH, ER, HE, AN
    for (uint32_t i = 0; i < m_solutions.size(); i++)
    {
        std::string::size_type n = 0;
        while ((n = m_solutions[i].word.find('1', n)) != std::string::npos)
        {
            m_solutions[i].word.replace(n, 1, "QU");
            n += 2;
        }
        n = 0;
        while ((n = m_solutions[i].word.find('2', n)) != std::string::npos)
        {
            m_solutions[i].word.replace(n, 1, "IN");
            n += 2;
        }
        n = 0;
        while ((n = m_solutions[i].word.find('3', n)) != std::string::npos)
        {
            m_solutions[i].word.replace(n, 1, "TH");
            n += 2;
        }
        n = 0;
        while ((n = m_solutions[i].word.find('4', n)) != std::string::npos)
        {
            m_solutions[i].word.replace(n, 1, "ER");
            n += 2;
        }
        n = 0;
        while ((n = m_solutions[i].word.find('5', n)) != std::string::npos)
        {
            m_solutions[i].word.replace(n, 1, "HE");
            n += 2;
        }
        n = 0;
        while ((n = m_solutions[i].word.find('6', n)) != std::string::npos)
        {
            m_solutions[i].word.replace(n, 1, "AN");
            n += 2;
        }
    }

    // Remove solutions of length 2 or less
    for (int32_t i = m_solutions.size() - 1; i >= 0; i--)
    {
        if (m_solutions[i].word.length() <= 2)
        {
            m_solutions.erase(m_solutions.begin() + i);
        }
    }

    // Sort alphabetically and then by length so that solutions of equal length are alphabetical
    std::sort(m_solutions.begin(), m_solutions.end(), [](Solver::Solution a, Solver::Solution b) { return a.word < b.word; });
    std::stable_sort(m_solutions.begin(), m_solutions.end(), [](Solver::Solution a, Solver::Solution b) { return a.word.size() > b.word.size(); });

    // Create containers
    m_solution_screen_grid_ptr = gtk_grid_new();

    m_solution_list_scrolling_ptr = gtk_scrolled_window_new(NULL, NULL);
    m_solution_list_box_ptr = gtk_list_box_new();

    m_solution_board_frame_ptr = gtk_aspect_frame_new("", 0.5, 0.5, 1.0, gtk_false());
    m_solution_board_overlay_ptr = gtk_overlay_new();
    m_solution_board_grid_ptr = gtk_grid_new();

    // Set solution screen grid properties
    gtk_grid_set_row_spacing(GTK_GRID(m_solution_screen_grid_ptr), 5.0);
    gtk_grid_set_column_spacing(GTK_GRID(m_solution_screen_grid_ptr), 5.0);
    gtk_grid_set_row_homogeneous(GTK_GRID(m_solution_screen_grid_ptr), gtk_true());
    gtk_grid_set_column_homogeneous(GTK_GRID(m_solution_screen_grid_ptr), gtk_true());

    // Set solution board frame properties
    gtk_frame_set_shadow_type(GTK_FRAME(m_solution_board_frame_ptr), GTK_SHADOW_NONE);
    gtk_container_set_border_width(GTK_CONTAINER(m_solution_board_frame_ptr), 20.0);

    // Set board grid properties
    gtk_grid_set_row_spacing(GTK_GRID(m_solution_board_grid_ptr), 5.0);
    gtk_grid_set_column_spacing(GTK_GRID(m_solution_board_grid_ptr), 5.0);
    gtk_grid_set_row_homogeneous(GTK_GRID(m_solution_board_grid_ptr), gtk_true());
    gtk_grid_set_column_homogeneous(GTK_GRID(m_solution_board_grid_ptr), gtk_true());

    // Connect row selection signal and set selected word
    g_signal_connect(G_OBJECT(m_solution_list_box_ptr), "row-selected", G_CALLBACK(On_Solution_Select), this);
    m_selected_solution = m_solutions.front();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(m_solution_list_box_ptr), GTK_SELECTION_SINGLE);

    // Add solutions to list
    for (uint16_t i = 0; i < m_solutions.size(); i++)
    {
        GtkWidget* list_row = gtk_list_box_row_new();
        GtkWidget* list_label = gtk_label_new(m_solutions[i].word.c_str());

        gtk_container_add(GTK_CONTAINER(list_row), list_label);
        gtk_container_add(GTK_CONTAINER(m_solution_list_box_ptr), list_row);
    }

    // Create font attributes
    PangoAttrList* font_attribute_list = pango_attr_list_new();
    PangoAttribute* font_size_attribute = pango_attr_size_new_absolute(PANGO_SCALE * 40.0);
    pango_attr_list_insert(font_attribute_list, font_size_attribute);

    // Create board display
    for (uint8_t x = 0; x < (*m_board_ptr)->Get_Board_Size(); x++)
    {
        for (uint8_t y = 0; y < (*m_board_ptr)->Get_Board_Size(); y++)
        {
            std::string die_value = (*m_board_ptr)->Get_Board_Cell(x, y);
            die_value = (die_value == "1") ? "Qu" : die_value;
            die_value = (die_value == "2") ? "In" : die_value;
            die_value = (die_value == "3") ? "Th" : die_value;
            die_value = (die_value == "4") ? "Er" : die_value;
            die_value = (die_value == "5") ? "He" : die_value;
            die_value = (die_value == "6") ? "An" : die_value;

            // Create label (using a button so that it has a border)
            GtkWidget* label = gtk_button_new_with_label(die_value.c_str());

            // Set label properties
            gtk_label_set_attributes(GTK_LABEL(gtk_bin_get_child(GTK_BIN(label))), font_attribute_list);
            gtk_widget_set_can_focus(label, gtk_false());

            // Add label to grid
            gtk_grid_attach(GTK_GRID(m_solution_board_grid_ptr), label, x, y, 1, 1);
        }
    }

    // Create drawing area
    m_solution_board_drawing_area_ptr = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(m_solution_board_drawing_area_ptr), "draw", G_CALLBACK(On_Solution_Path_Draw), this);

    // Structure top level container
    gtk_grid_attach(GTK_GRID(m_solution_screen_grid_ptr), m_solution_list_scrolling_ptr, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(m_solution_screen_grid_ptr), m_solution_board_frame_ptr, 1, 0, 2, 1);
    gtk_container_add(GTK_CONTAINER(m_window_ptr), m_solution_screen_grid_ptr);

    // Structure solution list
    gtk_container_add(GTK_CONTAINER(m_solution_list_scrolling_ptr), m_solution_list_box_ptr);

    // Structure board display
    gtk_overlay_add_overlay(GTK_OVERLAY(m_solution_board_overlay_ptr), m_solution_board_grid_ptr);
    gtk_overlay_add_overlay(GTK_OVERLAY(m_solution_board_overlay_ptr), m_solution_board_drawing_area_ptr);
    gtk_container_add(GTK_CONTAINER(m_solution_board_frame_ptr), m_solution_board_overlay_ptr);

    // Select first solution
    g_signal_emit_by_name(G_OBJECT(gtk_list_box_get_row_at_index(GTK_LIST_BOX(m_solution_list_box_ptr), 0)), "activate", this);

    // Show new widgets
    gtk_widget_show_all(m_window_ptr);
}

void Main_Interface::Destroy_Input_Screen()
{
    gtk_widget_destroy(m_input_screen_frame_ptr);

    m_input_screen_frame_ptr = nullptr;
    m_input_grid_ptr = nullptr;
    m_board_size_4x4_button_ptr = nullptr;
    m_board_size_5x5_button_ptr = nullptr;
    m_randomize_button_ptr = nullptr;
    m_solve_button_ptr = nullptr;
    m_entry_box_ptrs.clear();
}

bool Main_Interface::Is_String_Valid(const char* input, uint8_t board_size)
{
    if (board_size == 4)
    {
        const char* valid_strings[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "1"};

        for (uint8_t i = 0; i < 26; i++)
        {
            if (strcmp(input, valid_strings[i]) == 0)
            {
                return true;
            }
        }
    }
    else
    {
        const char* valid_strings[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6"};

        for (uint8_t i = 0; i < 31; i++)
        {
            if (strcmp(input, valid_strings[i]) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

void Main_Interface::Check_Inputs()
{
    m_are_inputs_valid = true;

    // Loop through cells
    for (uint8_t x = 0; x < (*m_board_ptr)->Get_Board_Size(); x++)
    {
        for (uint8_t y = 0; y < (*m_board_ptr)->Get_Board_Size(); y++)
        {
            // Test whether or not the input at cell x, y is valid
            m_are_inputs_valid &= Is_String_Valid((*m_board_ptr)->Get_Board_Cell(x, y).c_str(), (*m_board_ptr)->Get_Board_Size());
        }
    }
}

void Main_Interface::On_Input_Changed(GtkWidget* self, gpointer user_data)
{
    // Get input box text and data
    Input_Box_Callback_Data* input_box_callback_data_ptr = (Input_Box_Callback_Data*)user_data;
    std::string input_text = gtk_entry_get_text(GTK_ENTRY(self));

    // Convert input text to upper case
    std::transform(input_text.begin(), input_text.end(), input_text.begin(), ::toupper);

    input_text = (input_text == "QU") ? "1" : input_text;
    input_text = (input_text == "IN") ? "2" : input_text;
    input_text = (input_text == "TH") ? "3" : input_text;
    input_text = (input_text == "ER") ? "4" : input_text;
    input_text = (input_text == "HE") ? "5" : input_text;
    input_text = (input_text == "AN") ? "6" : input_text;

    // Remove added letter if it is not valid
    if (!Is_String_Valid(input_text.c_str(), (*input_box_callback_data_ptr->interface->m_board_ptr)->Get_Board_Size()) && input_text != "" && input_text != "Q")
    {
        input_text.pop_back();
    }

    // Set board cell to text
    (*input_box_callback_data_ptr->interface->m_board_ptr)->Set_Board_Cell(input_box_callback_data_ptr->x_position, input_box_callback_data_ptr->y_position, input_text);

    // Update text with processed text
    input_text = (input_text == "1") ? "Qu" : input_text;
    input_text = (input_text == "2") ? "In" : input_text;
    input_text = (input_text == "3") ? "Th" : input_text;
    input_text = (input_text == "4") ? "Er" : input_text;
    input_text = (input_text == "5") ? "He" : input_text;
    input_text = (input_text == "6") ? "An" : input_text;

    gtk_entry_set_text(GTK_ENTRY(self), input_text.c_str());

    // Enable / disable solve button
    input_box_callback_data_ptr->interface->Check_Inputs();
    gtk_widget_set_sensitive(input_box_callback_data_ptr->interface->m_solve_button_ptr, (gboolean)input_box_callback_data_ptr->interface->m_are_inputs_valid);
}

void Main_Interface::On_Input_Box_Destroy(GtkWidget* self, gpointer user_data)
{
    // Silence -Wunused-parameter (parameter is needed in callback signature)
    (void)self;

    // Free Input_Box_Callback_Data structure
    free((void*)user_data);
}

void Main_Interface::On_4x4_Clicked(GtkToggleButton* self, gpointer user_data)
{
    // Cast pointer to interface
    Main_Interface* interface_ptr = (Main_Interface*)user_data;

    // Determine whether 4x4 mode or 5x5 mode should be enabled
    bool is_4x4 = (bool)(gtk_toggle_button_get_active(self));

    // Preform actions based on current state of interface and buttons
    if (is_4x4 && !interface_ptr->m_is_4x4)
    {
        // Interface needs to be set to 4x4 mode
        free(*interface_ptr->m_board_ptr);
        *interface_ptr->m_board_ptr = new Board(4);
        interface_ptr->Destroy_Input_Screen();
        interface_ptr->Initialize_Input_Screen();
    }
    else if (!is_4x4 && interface_ptr->m_is_4x4)
    {
        // Keep interface at 4x4 and activate 4x4 button
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(interface_ptr->m_board_size_4x4_button_ptr), gtk_true());
    }
}

void Main_Interface::On_5x5_Clicked(GtkToggleButton* self, gpointer user_data)
{
    // Cast pointer to interface
    Main_Interface* interface_ptr = (Main_Interface*)user_data;

    // Determine whether 4x4 mode or 5x5 mode should be enabled
    bool is_5x5 = (bool)(gtk_toggle_button_get_active(self));

    // Preform actions based on current state of interface and buttons
    if (is_5x5 && interface_ptr->m_is_4x4)
    {
        // Interface needs to be set to 5x5 mode
        free(*interface_ptr->m_board_ptr);
        *interface_ptr->m_board_ptr = new Board(5);
        interface_ptr->Destroy_Input_Screen();
        interface_ptr->Initialize_Input_Screen();
    }
    else if (!is_5x5 && !interface_ptr->m_is_4x4)
    {
        // Keep interface at 5x5 and activate 5x5 button
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(interface_ptr->m_board_size_5x5_button_ptr), gtk_true());
    }
}

void Main_Interface::On_Randomized_Clicked(GtkWidget* self, gpointer user_data)
{
    // Silence -Wunused-parameter (parameter is needed in callback signature)
    (void)self;

    // Cast pointer to interface
    Main_Interface* interface_ptr = (Main_Interface*)user_data;

    // Randomize board
    (*interface_ptr->m_board_ptr)->Randomize();

    // Update text input boxes
    for (uint8_t x = 0; x < (*interface_ptr->m_board_ptr)->Get_Board_Size(); x++)
    {
        for (uint8_t y = 0; y < (*interface_ptr->m_board_ptr)->Get_Board_Size(); y++)
        {
            std::string cell_value = (*interface_ptr->m_board_ptr)->Get_Board_Cell(x, y);
            gtk_entry_set_text(GTK_ENTRY(interface_ptr->m_entry_box_ptrs[x][y]), cell_value.c_str());
        }
    }

    // Enable / disable solve button
    interface_ptr->Check_Inputs();
    gtk_widget_set_sensitive(interface_ptr->m_solve_button_ptr, (gboolean)interface_ptr->m_are_inputs_valid);
}

void Main_Interface::On_Solve_Clicked(GtkWidget* self, gpointer user_data)
{
    (void)self;                                                                                                                 // Silence -Wunused-parameter (parameter is needed in callback signature)
    Main_Interface* interface_ptr = (Main_Interface*)user_data;                                                                 // Cast pointer to interface
    interface_ptr->Destroy_Input_Screen();                                                                                      // Destroy input screen
    interface_ptr->m_solver_ptr = new Solver(interface_ptr->m_board_ptr, "../words/processed-collins-scrabble-words-2019.txt"); // Solve board
    interface_ptr->Initialize_Solution_Screen();                                                                                // Start solution screen
}

void Main_Interface::On_Solution_Select(GtkListBox* self, GtkListBoxRow* row, gpointer* user_data)
{
    // Silence -Wunused-parameter (parameter is needed in callback signature)
    (void)self;

    // Cast pointer to interface
    Main_Interface* interface_ptr = (Main_Interface*)user_data;

    // Set selected solution
    interface_ptr->m_selected_solution = interface_ptr->m_solutions[gtk_list_box_row_get_index(row)];

    // Refresh drawing area
    gtk_widget_queue_draw_area(interface_ptr->m_solution_board_drawing_area_ptr, 0, 0, gtk_widget_get_allocated_width(interface_ptr->m_solution_board_drawing_area_ptr),
                               gtk_widget_get_allocated_height(interface_ptr->m_solution_board_drawing_area_ptr));
}

void Main_Interface::On_Solution_Path_Draw(GtkWidget* drawing_area_ptr, cairo_t* cairo_ptr, gpointer data)
{
    // Cast pointer to interface
    Main_Interface* interface_ptr = (Main_Interface*)data;

    // Get drawing area dimensions
    uint32_t width = gtk_widget_get_allocated_width(drawing_area_ptr);
    uint32_t height = gtk_widget_get_allocated_height(drawing_area_ptr);

    // Set draw color
    cairo_set_source_rgba(cairo_ptr, 1.0, 1.0, 1.0, 0.2);

    // Set line width
    cairo_set_line_width(cairo_ptr, 10.0);

    // Go to starting position
    uint8_t x_position = interface_ptr->m_selected_solution.x_positions[0];
    uint8_t y_position = interface_ptr->m_selected_solution.y_positions[0];

    cairo_move_to(cairo_ptr, ((double)width / (*interface_ptr->m_board_ptr)->Get_Board_Size()) * x_position + ((double)width / (*interface_ptr->m_board_ptr)->Get_Board_Size() / 2.0),
                  ((double)height / (*interface_ptr->m_board_ptr)->Get_Board_Size()) * y_position + ((double)height / (*interface_ptr->m_board_ptr)->Get_Board_Size() / 2.0));

    // Loop through path
    for (uint32_t i = 1; i < interface_ptr->m_selected_solution.x_positions.size(); i++)
    {
        x_position = interface_ptr->m_selected_solution.x_positions[i];
        y_position = interface_ptr->m_selected_solution.y_positions[i];

        cairo_line_to(cairo_ptr, ((double)width / (*interface_ptr->m_board_ptr)->Get_Board_Size()) * x_position + ((double)width / (*interface_ptr->m_board_ptr)->Get_Board_Size() / 2.0),
                      ((double)height / (*interface_ptr->m_board_ptr)->Get_Board_Size()) * y_position + ((double)height / (*interface_ptr->m_board_ptr)->Get_Board_Size() / 2.0));
    }

    cairo_stroke(cairo_ptr);
}