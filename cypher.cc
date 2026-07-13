-- ============================================================
-- CORE LIBRARY
-- ============================================================

local CypherLib = {}
CypherLib.__index = CypherLib

-- ============================================================
-- UTILITY FUNCTIONS
-- ============================================================

local function clone(t)
    local copy = {}
    for k, v in pairs(t) do
        copy[k] = v
    end
    return copy
end

local function deepClone(t)
    local copy = {}
    for k, v in pairs(t) do
        if type(v) == "table" then
            copy[k] = deepClone(v)
        else
            copy[k] = v
        end
    end
    return copy
end

local function isColor3(c)
    return type(c) == "table" and type(c.R) == "number" and type(c.G) == "number" and type(c.B) == "number"
end

local function colorToHex(c)
    if not isColor3(c) then return "#FFFFFF" end
    local function toHex(n)
        local h = string.format("%x", math.floor(n * 255))
        return #h == 1 and "0" .. h or h
    end
    return "#" .. toHex(c.R) .. toHex(c.G) .. toHex(c.B)
end

local function hexToColor(hex)
    hex = hex:gsub("#", "")
    return Color3.fromRGB(
        tonumber("0x" .. hex:sub(1, 2)) or 255,
        tonumber("0x" .. hex:sub(3, 4)) or 255,
        tonumber("0x" .. hex:sub(5, 6)) or 255
    )
end

-- ============================================================
-- CLASS: CypherLib.UI (Main UI Framework)
-- ============================================================

local UILib = {}
UILib.__index = UILib

function UILib:new(config)
    local self = setmetatable({}, UILib)
    
    self.Name = config.Name or "Cypher.cc"
    self.LibraryName = config.LibraryName or "CypherLib"
    self.TitleMode = config.TitleMode or "Type" -- "Type" or "Static"
    self.ConfigFolder = config.ConfigFolder or "CypherConfig"
    self.Size = config.Size or UDim2.fromOffset(600, 540)
    self.ShowMobileButton = config.ShowMobileButton or false
    self.AutoDeviceCheck = config.AutoDeviceCheck or true
    
    self.Tabs = {}
    self.ActiveTab = nil
    self.Elements = {}
    self.Keybinds = {}
    self.WatermarkEnabled = false
    self.KeybindListEnabled = false
    self.Visible = true
    self.Mobile = false
    self.Dragging = false
    self.DragStart = nil
    
    -- Build UI
    self:BuildUI()
    
    return self
end

function UILib:BuildUI()
    -- Create ScreenGui
    self.Gui = Instance.new("ScreenGui")
    self.Gui.Name = self.Name .. "GUI"
    self.Gui.ResetOnSpawn = false
    self.Gui.Parent = game:GetService("CoreGui")
    self.Gui.ZIndexBehavior = Enum.ZIndexBehavior.Sibling
    
    -- Main Container
    self.Main = Instance.new("Frame")
    self.Main.Name = "MainFrame"
    self.Main.Size = self.Size
    self.Main.Position = UDim2.new(0.5, -self.Size.X.Offset / 2, 0.5, -self.Size.Y.Offset / 2)
    self.Main.BackgroundColor3 = Color3.fromRGB(25, 25, 35)
    self.Main.BackgroundTransparency = 0.05
    self.Main.BorderSizePixel = 1
    self.Main.BorderColor3 = Color3.fromRGB(60, 60, 80)
    self.Main.ClipsDescendants = true
    self.Main.Parent = self.Gui
    
    -- Title Bar (draggable)
    self.TitleBar = Instance.new("Frame")
    self.TitleBar.Name = "TitleBar"
    self.TitleBar.Size = UDim2.new(1, 0, 0, 40)
    self.TitleBar.BackgroundColor3 = Color3.fromRGB(35, 35, 45)
    self.TitleBar.BackgroundTransparency = 0.0
    self.TitleBar.BorderSizePixel = 0
    self.TitleBar.Parent = self.Main
    
    -- Title Text
    self.TitleText = Instance.new("TextLabel")
    self.TitleText.Name = "TitleText"
    self.TitleText.Size = UDim2.new(0.8, 0, 1, 0)
    self.TitleText.Position = UDim2.new(0, 10, 0, 0)
    self.TitleText.BackgroundTransparency = 1
    self.TitleText.TextColor3 = Color3.fromRGB(255, 255, 255)
    self.TitleText.Text = self.Name
    self.TitleText.TextXAlignment = Enum.TextXAlignment.Left
    self.TitleText.Font = Enum.Font.GothamBold
    self.TitleText.TextSize = 18
    self.TitleText.Parent = self.TitleBar
    
    -- Close Button
    self.CloseBtn = Instance.new("TextButton")
    self.CloseBtn.Name = "CloseBtn"
    self.CloseBtn.Size = UDim2.new(0, 30, 0, 30)
    self.CloseBtn.Position = UDim2.new(1, -35, 0, 5)
    self.CloseBtn.BackgroundColor3 = Color3.fromRGB(255, 70, 70)
    self.CloseBtn.BackgroundTransparency = 0.8
    self.CloseBtn.BorderSizePixel = 0
    self.CloseBtn.Text = "✕"
    self.CloseBtn.TextColor3 = Color3.fromRGB(255, 255, 255)
    self.CloseBtn.TextSize = 16
    self.CloseBtn.Font = Enum.Font.GothamBold
    self.CloseBtn.Parent = self.TitleBar
    
    self.CloseBtn.MouseButton1Click:Connect(function()
        self:Unload()
    end)
    
    -- Drag functionality
    local function startDrag(input)
        self.Dragging = true
        self.DragStart = input.Position - self.Main.Position
        self.DragStart = UDim2.new(0, self.DragStart.X.Offset, 0, self.DragStart.Y.Offset)
    end
    
    local function updateDrag(input)
        if self.Dragging then
            local newPos = input.Position - self.DragStart
            self.Main.Position = UDim2.new(0, newPos.X.Offset, 0, newPos.Y.Offset)
        end
    end
    
    local function endDrag()
        self.Dragging = false
    end
    
    self.TitleBar.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            startDrag(input)
        end
    end)
    
    self.TitleBar.InputChanged:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch then
            updateDrag(input)
        end
    end)
    
    self.TitleBar.InputEnded:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            endDrag()
        end
    end)
    
    -- Tab Container
    self.TabContainer = Instance.new("Frame")
    self.TabContainer.Name = "TabContainer"
    self.TabContainer.Size = UDim2.new(1, 0, 0, 50)
    self.TabContainer.Position = UDim2.new(0, 0, 0, 40)
    self.TabContainer.BackgroundColor3 = Color3.fromRGB(30, 30, 40)
    self.TabContainer.BackgroundTransparency = 0.0
    self.TabContainer.BorderSizePixel = 0
    self.TabContainer.Parent = self.Main
    
    -- Tab Scrolling Frame
    self.TabScroller = Instance.new("ScrollingFrame")
    self.TabScroller.Name = "TabScroller"
    self.TabScroller.Size = UDim2.new(1, -10, 1, 0)
    self.TabScroller.Position = UDim2.new(0, 5, 0, 0)
    self.TabScroller.BackgroundTransparency = 1
    self.TabScroller.BorderSizePixel = 0
    self.TabScroller.CanvasSize = UDim2.new(0, 0, 0, 0)
    self.TabScroller.ScrollBarThickness = 0
    self.TabScroller.Parent = self.TabContainer
    
    -- Main Content Area
    self.Content = Instance.new("Frame")
    self.Content.Name = "Content"
    self.Content.Size = UDim2.new(1, 0, 1, -90)
    self.Content.Position = UDim2.new(0, 0, 0, 90)
    self.Content.BackgroundTransparency = 1
    self.Content.BorderSizePixel = 0
    self.Content.ClipsDescendants = true
    self.Content.Parent = self.Main
    
    -- Watermark
    self.WatermarkLabel = Instance.new("TextLabel")
    self.WatermarkLabel.Name = "Watermark"
    self.WatermarkLabel.Size = UDim2.new(0, 200, 0, 20)
    self.WatermarkLabel.Position = UDim2.new(0, 10, 1, -25)
    self.WatermarkLabel.BackgroundTransparency = 1
    self.WatermarkLabel.TextColor3 = Color3.fromRGB(150, 150, 170)
    self.WatermarkLabel.Text = self.Name .. " v2.0 | " .. game.Players.LocalPlayer.Name
    self.WatermarkLabel.TextXAlignment = Enum.TextXAlignment.Left
    self.WatermarkLabel.Font = Enum.Font.Gotham
    self.WatermarkLabel.TextSize = 12
    self.WatermarkLabel.Visible = false
    self.WatermarkLabel.Parent = self.Main
    
    -- Keybind List
    self.KeybindListFrame = Instance.new("Frame")
    self.KeybindListFrame.Name = "KeybindList"
    self.KeybindListFrame.Size = UDim2.new(0, 200, 0, 0)
    self.KeybindListFrame.Position = UDim2.new(1, -210, 0, 50)
    self.KeybindListFrame.BackgroundColor3 = Color3.fromRGB(20, 20, 30)
    self.KeybindListFrame.BackgroundTransparency = 0.2
    self.KeybindListFrame.BorderSizePixel = 1
    self.KeybindListFrame.BorderColor3 = Color3.fromRGB(60, 60, 80)
    self.KeybindListFrame.ClipsDescendants = true
    self.KeybindListFrame.Visible = false
    self.KeybindListFrame.Parent = self.Main
    
    -- Keybind List Title
    local kblTitle = Instance.new("TextLabel")
    kblTitle.Name = "Title"
    kblTitle.Size = UDim2.new(1, 0, 0, 25)
    kblTitle.BackgroundTransparency = 1
    kblTitle.TextColor3 = Color3.fromRGB(255, 255, 255)
    kblTitle.Text = "Keybinds"
    kblTitle.Font = Enum.Font.GothamBold
    kblTitle.TextSize = 14
    kblTitle.Parent = self.KeybindListFrame
    
    self.KeybindListContent = Instance.new("ScrollingFrame")
    self.KeybindListContent.Name = "Content"
    self.KeybindListContent.Size = UDim2.new(1, 0, 1, -25)
    self.KeybindListContent.Position = UDim2.new(0, 0, 0, 25)
    self.KeybindListContent.BackgroundTransparency = 1
    self.KeybindListContent.BorderSizePixel = 0
    self.KeybindListContent.CanvasSize = UDim2.new(0, 0, 0, 0)
    self.KeybindListContent.ScrollBarThickness = 3
    self.KeybindListContent.Parent = self.KeybindListFrame
    
    -- Auto device check
    if self.AutoDeviceCheck then
        local isMobile = game:GetService("UserInputService").TouchEnabled
        self.Mobile = isMobile
        if isMobile and not self.ShowMobileButton then
            self.Main.Size = UDim2.fromOffset(400, 480)
            self.Main.Position = UDim2.new(0.5, -200, 0.5, -240)
        end
    end
    
    -- Typewriter effect
    if self.TitleMode == "Type" then
        self:StartTypewriter()
    end
    
    -- Mobile toggle button
    if self.ShowMobileButton then
        local mobBtn = Instance.new("TextButton")
        mobBtn.Size = UDim2.new(0, 30, 0, 30)
        mobBtn.Position = UDim2.new(1, -70, 0, 5)
        mobBtn.BackgroundColor3 = Color3.fromRGB(60, 60, 80)
        mobBtn.BackgroundTransparency = 0.5
        mobBtn.BorderSizePixel = 0
        mobBtn.Text = "📱"
        mobBtn.TextColor3 = Color3.fromRGB(255, 255, 255)
        mobBtn.TextSize = 16
        mobBtn.Font = Enum.Font.Gotham
        mobBtn.Parent = self.TitleBar
        
        mobBtn.MouseButton1Click:Connect(function()
            self.Mobile = not self.Mobile
            if self.Mobile then
                self.Main.Size = UDim2.fromOffset(400, 480)
                self.Main.Position = UDim2.new(0.5, -200, 0.5, -240)
            else
                self.Main.Size = self.Size
                self.Main.Position = UDim2.new(0.5, -self.Size.X.Offset / 2, 0.5, -self.Size.Y.Offset / 2)
            end
        end)
    end
    
    -- Initial tab
    self:SelectTab(nil)
end

function UILib:StartTypewriter()
    local text = self.Name
    local index = 0
    self.TitleText.Text = ""
    
    local function typeChar()
        if index < #text then
            index = index + 1
            self.TitleText.Text = text:sub(1, index)
            task.wait(0.05)
            typeChar()
        else
            self.TitleText.Text = text
        end
    end
    
    task.spawn(typeChar)
end

-- ============================================================
-- TAB / CATEGORY SYSTEM
-- ============================================================

function UILib:AddCategory(config)
    local name = config.Name or "Tab"
    local tab = {
        Name = name,
        Sections = {},
        ActiveSection = nil,
        Button = nil,
        Content = nil,
        Parent = self,
    }
    
    -- Tab button
    local btn = Instance.new("TextButton")
    btn.Name = name .. "TabBtn"
    btn.Size = UDim2.new(0, 100, 1, 0)
    btn.BackgroundTransparency = 1
    btn.Text = name
    btn.TextColor3 = Color3.fromRGB(200, 200, 220)
    btn.TextSize = 14
    btn.Font = Enum.Font.Gotham
    btn.Parent = self.TabScroller
    
    -- Update canvas size
    local buttons = self.TabScroller:GetChildren()
    local totalWidth = 0
    for _, child in ipairs(buttons) do
        if child:IsA("TextButton") then
            totalWidth = totalWidth + 100
        end
    end
    self.TabScroller.CanvasSize = UDim2.new(0, totalWidth, 0, 0)
    
    -- Tab content container
    local content = Instance.new("Frame")
    content.Name = name .. "Content"
    content.Size = UDim2.new(1, 0, 1, 0)
    content.BackgroundTransparency = 1
    content.BorderSizePixel = 0
    content.Visible = false
    content.Parent = self.Content
    
    tab.Button = btn
    tab.Content = content
    
    btn.MouseButton1Click:Connect(function()
        self:SelectTab(tab)
    end)
    
    table.insert(self.Tabs, tab)
    
    -- If this is the first tab, select it
    if #self.Tabs == 1 then
        self:SelectTab(tab)
    end
    
    -- Return an object for adding sections
    local tabAPI = {
        AddSection = function(sectionConfig)
            return self:AddSection(tab, sectionConfig)
        end,
        _tab = tab
    }
    
    return tabAPI
end

function UILib:SelectTab(tab)
    -- Deselect all
    for _, t in ipairs(self.Tabs) do
        t.Button.BackgroundTransparency = 1
        t.Button.TextColor3 = Color3.fromRGB(200, 200, 220)
        t.Content.Visible = false
    end
    
    if tab then
        tab.Button.BackgroundColor3 = Color3.fromRGB(60, 60, 80)
        tab.Button.BackgroundTransparency = 0.8
        tab.Button.TextColor3 = Color3.fromRGB(255, 255, 255)
        tab.Content.Visible = true
        self.ActiveTab = tab
    end
end

-- ============================================================
-- SECTION SYSTEM
-- ============================================================

function UILib:AddSection(tab, config)
    local name = config.Name or "Section"
    local columns = config.Columns or 1
    columns = math.clamp(columns, 1, 3)
    
    local section = {
        Name = name,
        Columns = columns,
        Pages = {},
        Parent = tab,
        Container = nil,
        Title = nil,
    }
    
    -- Section container
    local container = Instance.new("Frame")
    container.Name = name .. "Section"
    container.Size = UDim2.new(1, -20, 0, 50)
    container.Position = UDim2.new(0, 10, 0, 10)
    container.BackgroundColor3 = Color3.fromRGB(30, 30, 40)
    container.BackgroundTransparency = 0.3
    container.BorderSizePixel = 1
    container.BorderColor3 = Color3.fromRGB(50, 50, 65)
    container.ClipsDescendants = true
    container.Parent = tab.Content
    
    -- Section title
    local title = Instance.new("TextLabel")
    title.Name = "Title"
    title.Size = UDim2.new(1, 0, 0, 25)
    title.BackgroundTransparency = 1
    title.TextColor3 = Color3.fromRGB(220, 220, 240)
    title.Text = name
    title.TextXAlignment = Enum.TextXAlignment.Left
    title.Font = Enum.Font.GothamBold
    title.TextSize = 14
    title.Parent = container
    
    -- Content area
    local content = Instance.new("Frame")
    content.Name = "Content"
    content.Size = UDim2.new(1, 0, 1, -25)
    content.Position = UDim2.new(0, 0, 0, 25)
    content.BackgroundTransparency = 1
    content.BorderSizePixel = 0
    content.Parent = container
    
    -- Create column frames
    local columnFrames = {}
    for i = 1, columns do
        local col = Instance.new("Frame")
        col.Name = "Column" .. i
        col.Size = UDim2.new(1 / columns, -5 * (columns - 1), 1, 0)
        col.Position = UDim2.new((i - 1) / columns, 5 * (i - 1), 0, 0)
        col.BackgroundTransparency = 1
        col.BorderSizePixel = 0
        col.Parent = content
        table.insert(columnFrames, col)
    end
    
    section.Container = container
    section.Title = title
    section._columnFrames = columnFrames
    
    -- Store with auto-sizing
    local function updateSize()
        local totalHeight = 50
        for _, child in ipairs(container.Content:GetChildren()) do
            if child:IsA("Frame") then
                for _, grandchild in ipairs(child:GetChildren()) do
                    if grandchild:IsA("Frame") or grandchild:IsA("ScrollingFrame") then
                        local posY = grandchild.Position.Y.Offset + grandchild.Size.Y.Offset
                        if posY + 20 > totalHeight then
                            totalHeight = posY + 20
                        end
                    end
                end
            end
        end
        container.Size = UDim2.new(1, -20, 0, totalHeight)
    end
    
    section._updateSize = updateSize
    
    table.insert(tab.Sections, section)
    
    -- Return API
    local sectionAPI = {
        AddPage = function(pageConfig)
            return self:AddPage(section, pageConfig)
        end,
        _section = section
    }
    
    return sectionAPI
end

-- ============================================================
-- PAGE / GROUP SYSTEM
-- ============================================================

function UILib:AddPage(section, config)
    local name = config.Name or "Page"
    local description = config.Description or ""
    local side = config.Side or 1
    side = math.clamp(side, 1, section.Columns)
    
    local page = {
        Name = name,
        Description = description,
        Side = side,
        Elements = {},
        Container = nil,
        Parent = section,
    }
    
    -- Page container
    local container = Instance.new("Frame")
    container.Name = name .. "Page"
    container.Size = UDim2.new(1, 0, 0, 50)
    container.BackgroundTransparency = 1
    container.BorderSizePixel = 0
    container.Parent = section._columnFrames[side]
    
    -- Page title
    local title = Instance.new("TextLabel")
    title.Name = "Title"
    title.Size = UDim2.new(1, 0, 0, 20)
    title.BackgroundTransparency = 1
    title.TextColor3 = Color3.fromRGB(200, 200, 220)
    title.Text = name
    title.TextXAlignment = Enum.TextXAlignment.Left
    title.Font = Enum.Font.GothamBold
    title.TextSize = 13
    title.Parent = container
    
    -- Description
    local desc = nil
    if description ~= "" then
        desc = Instance.new("TextLabel")
        desc.Name = "Description"
        desc.Size = UDim2.new(1, 0, 0, 15)
        desc.Position = UDim2.new(0, 0, 0, 20)
        desc.BackgroundTransparency = 1
        desc.TextColor3 = Color3.fromRGB(150, 150, 180)
        desc.Text = description
        desc.TextXAlignment = Enum.TextXAlignment.Left
        desc.Font = Enum.Font.Gotham
        desc.TextSize = 11
        desc.Parent = container
    end
    
    -- Elements container
    local elementsContainer = Instance.new("Frame")
    elementsContainer.Name = "Elements"
    elementsContainer.Size = UDim2.new(1, 0, 0, 0)
    elementsContainer.Position = UDim2.new(0, 0, 0, (description ~= "" and 40 or 25))
    elementsContainer.BackgroundTransparency = 1
    elementsContainer.BorderSizePixel = 0
    elementsContainer.Parent = container
    
    page.Container = container
    page._elementsContainer = elementsContainer
    
    -- Update container height function
    local function updateHeight()
        local totalHeight = (description ~= "" and 40 or 25)
        for _, child in ipairs(elementsContainer:GetChildren()) do
            if child:IsA("Frame") then
                local posY = child.Position.Y.Offset + child.Size.Y.Offset
                if posY > totalHeight then
                    totalHeight = posY
                end
            end
        end
        container.Size = UDim2.new(1, 0, 0, totalHeight + 10)
    end
    
    page._updateHeight = updateHeight
    
    table.insert(section.Pages, page)
    
    -- Return API for adding elements
    local pageAPI = {
        AddToggle = function(toggleConfig)
            return self:AddToggle(page, toggleConfig)
        end,
        AddSlider = function(sliderConfig)
            return self:AddSlider(page, sliderConfig)
        end,
        AddDropdown = function(dropdownConfig)
            return self:AddDropdown(page, dropdownConfig)
        end,
        AddColorpicker = function(colorConfig)
            return self:AddColorpicker(page, colorConfig)
        end,
        AddKeybind = function(keybindConfig)
            return self:AddKeybind(page, keybindConfig)
        end,
        AddKeybindToggle = function(keybindConfig)
            return self:AddKeybindToggle(page, keybindConfig)
        end,
        AddButton = function(buttonConfig)
            return self:AddButton(page, buttonConfig)
        end,
        AddParagraph = function(paragraphConfig)
            return self:AddParagraph(page, paragraphConfig)
        end,
        _page = page
    }
    
    return pageAPI
end

-- ============================================================
-- UI ELEMENTS
-- ============================================================

-- -------------------- TOGGLE --------------------

function UILib:AddToggle(page, config)
    local name = config.Name or "Toggle"
    local default = config.Default or false
    local flag = config.Flag or ""
    local callback = config.Callback or function() end
    
    local element = {
        Type = "Toggle",
        Name = name,
        Value = default,
        Flag = flag,
        Callback = callback,
        Container = nil,
        ToggleBtn = nil,
        Label = nil,
    }
    
    -- Container
    local container = Instance.new("Frame")
    container.Name = name .. "Toggle"
    container.Size = UDim2.new(1, 0, 0, 25)
    container.BackgroundTransparency = 1
    container.BorderSizePixel = 0
    container.Parent = page._elementsContainer
    
    -- Label
    local label = Instance.new("TextLabel")
    label.Name = "Label"
    label.Size = UDim2.new(0.7, 0, 1, 0)
    label.BackgroundTransparency = 1
    label.TextColor3 = Color3.fromRGB(200, 200, 220)
    label.Text = name
    label.TextXAlignment = Enum.TextXAlignment.Left
    label.Font = Enum.Font.Gotham
    label.TextSize = 13
    label.Parent = container
    
    -- Toggle button
    local toggle = Instance.new("TextButton")
    toggle.Name = "ToggleBtn"
    toggle.Size = UDim2.new(0, 40, 0, 20)
    toggle.Position = UDim2.new(1, -45, 0.5, -10)
    toggle.BackgroundColor3 = default and Color3.fromRGB(80, 200, 80) or Color3.fromRGB(60, 60, 80)
    toggle.BackgroundTransparency = 0.5
    toggle.BorderSizePixel = 0
    toggle.Text = default and "ON" or "OFF"
    toggle.TextColor3 = Color3.fromRGB(255, 255, 255)
    toggle.TextSize = 10
    toggle.Font = Enum.Font.GothamBold
    toggle.Parent = container
    
    -- State
    local state = default
    
    local function setValue(value)
        state = value
        toggle.BackgroundColor3 = value and Color3.fromRGB(80, 200, 80) or Color3.fromRGB(60, 60, 80)
        toggle.Text = value and "ON" or "OFF"
        element.Value = value
        callback(value)
        
        -- Save to config system if flag exists
        if flag ~= "" and self._configSystem then
            self._configSystem:SaveValue(flag, value)
        end
    end
    
    toggle.MouseButton1Click:Connect(function()
        setValue(not state)
    end)
    
    element.Container = container
    element.ToggleBtn = toggle
    element.Label = label
    element._setValue = setValue
    element._getValue = function() return state end
    
    table.insert(page.Elements, element)
    page._updateHeight()
    
    -- Register flag
    if flag ~= "" and self._configSystem then
        self._configSystem:RegisterElement(flag, element)
    end
    
    -- Return API
    return {
        SetValue = setValue,
        GetValue = function() return state end,
        _element = element
    }
end

-- -------------------- SLIDER --------------------

function UILib:AddSlider(page, config)
    local name = config.Name or "Slider"
    local min = config.Min or 0
    local max = config.Max or 100
    local default = config.Default or 50
    local step = config.Step or 1
    local precise = config.Precise or false
    local flag = config.Flag or ""
    local callback = config.Callback or function() end
    
    local element = {
        Type = "Slider",
        Name = name,
        Min = min,
        Max = max,
        Value = default,
        Step = step,
        Precise = precise,
        Flag = flag,
        Callback = callback,
        Container = nil,
        SliderBar = nil,
        SliderFill = nil,
        SliderHandle = nil,
        ValueLabel = nil,
        Label = nil,
    }
    
    -- Container
    local container = Instance.new("Frame")
    container.Name = name .. "Slider"
    container.Size = UDim2.new(1, 0, 0, 40)
    container.BackgroundTransparency = 1
    container.BorderSizePixel = 0
    container.Parent = page._elementsContainer
    
    -- Label
    local label = Instance.new("TextLabel")
    label.Name = "Label"
    label.Size = UDim2.new(0.6, 0, 0, 18)
    label.BackgroundTransparency = 1
    label.TextColor3 = Color3.fromRGB(200, 200, 220)
    label.Text = name
    label.TextXAlignment = Enum.TextXAlignment.Left
    label.Font = Enum.Font.Gotham
    label.TextSize = 13
    label.Parent = container
    
    -- Value label
    local valueLabel = Instance.new("TextLabel")
    valueLabel.Name = "Value"
    valueLabel.Size = UDim2.new(0.4, 0, 0, 18)
    valueLabel.Position = UDim2.new(0.6, 0, 0, 0)
    valueLabel.BackgroundTransparency = 1
    valueLabel.TextColor3 = Color3.fromRGB(220, 220, 240)
    valueLabel.Text = precise and string.format("%.1f", default) or tostring(default)
    valueLabel.TextXAlignment = Enum.TextXAlignment.Right
    valueLabel.Font = Enum.Font.Gotham
    valueLabel.TextSize = 13
    valueLabel.Parent = container
    
    -- Slider track
    local sliderBar = Instance.new("Frame")
    sliderBar.Name = "SliderBar"
    sliderBar.Size = UDim2.new(1, 0, 0, 4)
    sliderBar.Position = UDim2.new(0, 0, 0, 28)
    sliderBar.BackgroundColor3 = Color3.fromRGB(50, 50, 65)
    sliderBar.BackgroundTransparency = 0.5
    sliderBar.BorderSizePixel = 0
    sliderBar.Parent = container
    
    -- Slider fill
    local sliderFill = Instance.new("Frame")
    sliderFill.Name = "SliderFill"
    sliderFill.Size = UDim2.new((default - min) / (max - min), 0, 1, 0)
    sliderFill.BackgroundColor3 = Color3.fromRGB(80, 200, 200)
    sliderFill.BackgroundTransparency = 0.0
    sliderFill.BorderSizePixel = 0
    sliderFill.Parent = sliderBar
    
    -- Slider handle
    local sliderHandle = Instance.new("Frame")
    sliderHandle.Name = "SliderHandle"
    sliderHandle.Size = UDim2.new(0, 12, 0, 12)
    sliderHandle.Position = UDim2.new((default - min) / (max - min), -6, 0.5, -6)
    sliderHandle.BackgroundColor3 = Color3.fromRGB(100, 220, 220)
    sliderHandle.BackgroundTransparency = 0.0
    sliderHandle.BorderSizePixel = 1
    sliderHandle.BorderColor3 = Color3.fromRGB(150, 150, 180)
    sliderHandle.Parent = sliderBar
    
    -- State
    local state = default
    
    local function setValue(value)
        value = math.clamp(value, min, max)
        if step and step > 0 then
            local rounded = math.floor((value - min) / step + 0.5) * step + min
            value = rounded
        end
        state = value
        local percentage = (value - min) / (max - min)
        sliderFill.Size = UDim2.new(percentage, 0, 1, 0)
        sliderHandle.Position = UDim2.new(percentage, -6, 0.5, -6)
        valueLabel.Text = precise and string.format("%.1f", value) or tostring(math.floor(value))
        element.Value = value
        callback(value)
        
        if flag ~= "" and self._configSystem then
            self._configSystem:SaveValue(flag, value)
        end
    end
    
    -- Slider interaction
    local function onSliderClick(input)
        local position = input.Position.X.Offset
        local barWidth = sliderBar.AbsoluteSize.X
        if barWidth <= 0 then return end
        local percentage = math.clamp(position / barWidth, 0, 1)
        local value = min + percentage * (max - min)
        setValue(value)
    end
    
    sliderBar.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
            onSliderClick(input)
        end
    end)
    
    sliderBar.InputChanged:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch then
            if input.Position.X.Offset > 0 then
                onSliderClick(input)
            end
        end
    end)
    
