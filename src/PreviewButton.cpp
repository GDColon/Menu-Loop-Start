// stole this from the geode docs lol
// this is all just to have a "preview" button in the settings

#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

class PreviewButtonSetting : public SettingV3 {
public:
    // Once again implement the parse function
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
        auto res = std::make_shared<PreviewButtonSetting>();
        auto root = checkJson(json, "PreviewButtonSetting");

        res->init(key, modID, root);
        res->parseNameAndDescription(root);
        res->parseEnableIf(root);
        
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    bool load(matjson::Value const& json) override { return true; }
    bool save(matjson::Value& json) const override { return true; }
    bool isDefaultValue() const override { return true; }
    void reset() override {}

    SettingNodeV3* createNode(float width) override;
};

// We are inheriting from `SettingNodeV3` directly again, as we don't need the 
// boilerplate `SettingValueNodeV3` fills in for us because our setting has no 
// value!
class PreviewButton : public SettingNodeV3 {
protected:
    ButtonSprite* m_buttonSprite;
    CCMenuItemSpriteExtra* m_button;

    bool init(std::shared_ptr<PreviewButtonSetting> setting, float width) {
        if (!SettingNodeV3::init(setting, width))
            return false;
    
        // Button is created here
        m_buttonSprite = ButtonSprite::create("Click to preview!", "goldFont.fnt", "geode.loader/GE_button_05.png", .8f);
        m_buttonSprite->setScale(0.6f);
        m_button = CCMenuItemSpriteExtra::create(
            m_buttonSprite, this, menu_selector(PreviewButton::onButton)
        );
        this->getButtonMenu()->addChildAtPosition(m_button, Anchor::Center);
        this->getButtonMenu()->setContentWidth(120);
        this->getButtonMenu()->updateLayout();
        this->updateState(nullptr);
        return true;
    }
    
    void updateState(CCNode* invoker) override {
        SettingNodeV3::updateState(invoker);
    }

    void onButton(CCObject*) {
        FMODAudioEngine::sharedEngine()->stopAndRemoveMusic(0);
        GameManager::get()->fadeInMenuMusic();
    }

    void onCommit() override {}
    void onResetToDefault() override {}

public:
    static PreviewButton* create(std::shared_ptr<PreviewButtonSetting> setting, float width) {
        auto ret = new PreviewButton();
        if (ret && ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool hasUncommittedChanges() const override { return false; }
    bool hasNonDefaultValue() const override { return false; }
};

// Create node as before
SettingNodeV3* PreviewButtonSetting::createNode(float width) {
    return PreviewButton::create(
        std::static_pointer_cast<PreviewButtonSetting>(shared_from_this()),
        width
    );
}

// Register as before
$execute {
    (void)Mod::get()->registerCustomSettingType("preview", &PreviewButtonSetting::parse);
}
