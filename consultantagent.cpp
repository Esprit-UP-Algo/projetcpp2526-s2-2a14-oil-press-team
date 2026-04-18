#include "consultantagent.h"
#include <QRegularExpression>

ConsultantAgent::ConsultantAgent(QObject *parent) : QObject(parent)
{
    initializeKnowledge();
}

void ConsultantAgent::initializeKnowledge()
{
    knowledgeBase["machinery"] = {"Extraction Machinery", "Premium Industrial (Italy/Sweden)", "Varies by scale", 
        "For 2026, **Pieralisi (Italy)** and **Alfa Laval (Sweden)** remain the gold standard for high-capacity mills. "
        "Their new 'EcoLine' systems focus on water conservation and 10% higher polyphenol preservation. "
        "For small boutique producers, the trend is **mobile stainless steel presses** (30-200kg/h) which ensure immediate milling for superior quality."};
    
    knowledgeBase["bottles"] = {"Glass Packaging", "Premium Dark Glass (UV Protection)", "$0.75 - $3.00 / unit", 
        "Best choices for 2026 are **Verallia** or **Vetroelit** dark glass. Prices range from $0.75 for standard bulk to $3.00 for custom premium designs. "
        "Always recommend dark green or amber to protect the oil from light degradation. Heavy glass is trending to justify 'Premium' pricing."};
    
    knowledgeBase["fertilizer"] = {"Agricultural Inputs (NPK/Urea)", "Agronomic Efficiency focus", "Urea >$800/ton", 
        "Fertilizer prices are currently elevated. **Urea** is crossing $800/ton. I recommend focusing on **Ternary NPK fertilizers** (currently €620-650/ton). "
        "The best ROI in 2026 comes from 'Precision Application'—using IoT soil sensors to reduce waste by only applying where needed."};
    
    knowledgeBase["prices"] = {"Market Prices 2026", "Global Stability", "High volatility", 
        "Extra Virgin Olive Oil prices are stabilizing but remain high. Mediterranean harvest is projected at 3.4M tonnes. "
        "The best prices for raw materials are coming from **Tunisia and Italy** this year, while Spain is still recovering from climate stress."};

    knowledgeBase["deals"] = {"Exclusive Deals 2026", "Factory-Direct & Bulk Offers", "Up to 30% Savings", 
        "Current Top Deal: **Pieralisi Artisanal Bundle** (Save 15% on EcoLine 30 when bought with a maintenance contract). "
        "Packaging Deal: **Burch Bottle Bulk Discount** ($0.65 per unit for orders >5000 'Dorica' bottles). "
        "Agricultural Alert: **Pre-pay NPK Fertilizers** now to lock in currently low €620/ton rates before the summer energy surge."};

    knowledgeBase["tunisia"] = {"Tunisia Market 2026", "Major Harvest Expansion", "Projected 3.2M Tonnes", 
        "Tunisia is currently the most competitive raw material source for 2026. The expansion into the Sfax region has yielded record numbers. "
        "**Best Strategy:** Secure bulk olive oil supply contracts from Tunisian cooperatives now (April-May) before the European summer demand spikes."};

    knowledgeBase["italy"] = {"Italy Production 2026", "Puglia/Sicily Recovery", "High Premium Value", 
        "Italy is focusing on 'High-Polyphenol' certification this year. While volume in Puglia is recovering from Xylella, "
        "the Sicilia sub-market is offering high-end labels with 20% growth. Good for artisanal high-margin products."};

    knowledgeBase["pef"] = {"PEF (Pulsed Electric Field)", "Extraction Tech Disruptor", "5-10% Yield Increase", 
        "PEF technology uses electric pulses to open olive cells before pressing. Results show 20% higher antioxidant retention "
        "and reduced malaxation time. Recommended for mills targeting the 'Super-Premium' health market segment."};

    knowledgeBase["harvest"] = {"Harvesting Timing 2026", "Quality vs Quantity", "Strategic Window", 
        "For 2026, **Early Harvest (Green)** typically occurs in late September. It yields less oil but attracts a 40% price premium. "
        "**Late Harvest (Ripe)** in November yields maximum volume but lower sensory scores. I recommend a 30/70 split to balance portfolio ROI."};
}

QString ConsultantAgent::getResponse(const QString &userInput)
{
    QString input = userInput.toLower();
    
    if (input.contains("hello") || input.contains("hi") || input.contains("who are you")) {
        return "Hello! I am your OilPress Market Consultant. I can help you with advice on **Machinery**, **Packaging**, **Tunisia/Italy Markets**, **PEF Technology**, or **Harvest Strategies**. What's on your mind?";
    }

    if (input.contains("tunisia") || input.contains("sfax") || input.contains("tu nis")) {
        ProductInfo p = knowledgeBase["tunisia"];
        return QString("### %1\n**Context:** %2\n**Volume:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }

    if (input.contains("italy") || input.contains("puglia") || input.contains("sicily")) {
        ProductInfo p = knowledgeBase["italy"];
        return QString("### %1\n**Focus:** %2\n**Trend:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }

    if (input.contains("pef") || input.contains("electric pulse") || input.contains("pulse")) {
        ProductInfo p = knowledgeBase["pef"];
        return QString("### %1\n**Tech Status:** %2\n**Yield Impact:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }

    if (input.contains("harvest") || input.contains("gather") || input.contains("timing") || input.contains("september") || input.contains("november")) {
        ProductInfo p = knowledgeBase["harvest"];
        return QString("### %1\n**Factor:** %2\n**Strategic Value:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }
    
    if (input.contains("machinery") || input.contains("machine") || input.contains("extract") || input.contains("press")) {
        ProductInfo p = knowledgeBase["machinery"];
        return QString("### %1\n**Quality:** %2\n**Price Trend:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }
    
    if (input.contains("bottle") || input.contains("packaging") || input.contains("glass")) {
        ProductInfo p = knowledgeBase["bottles"];
        return QString("### %1\n**Quality:** %2\n**Price Trend:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }
    
    if (input.contains("fertilizer") || input.contains("npk") || input.contains("urea")) {
        ProductInfo p = knowledgeBase["fertilizer"];
        return QString("### %1\n**Quality:** %2\n**Price Trend:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }
    
    if (input.contains("deal") || input.contains("cheap") || input.contains("buy") || input.contains("discount") || input.contains("offer")) {
        ProductInfo p = knowledgeBase["deals"];
        return QString("### %1\n**Category:** %2\n**Offer:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }
    
    if (input.contains("price") || input.contains("market") || input.contains("cost") || input.contains("worth") || input.contains("sell")) {
        ProductInfo p = knowledgeBase["prices"];
        return QString("### %1\n**Market Trend:** %2\n**Details:** %3\n\n%4").arg(p.name, p.quality, p.price, p.summary);
    }
    
    return "That's an interesting question. While I specialize in Industry Machinery, Packaging, and Fertilizers, I can tell you that for anything in the olive oil sector, **quality always commands a 20-30% price premium** in the 2026 market. Could you specify if you are looking for equipment or raw material advice?";
}
