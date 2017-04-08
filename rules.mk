

$(OBJDIR)%.o: $(PROJDIR)%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@


$(OBJDIR)%.o: $(PROJDIR)%.c
	mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@


