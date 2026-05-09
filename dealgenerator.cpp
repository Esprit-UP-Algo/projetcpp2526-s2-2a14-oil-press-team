#include "dealgenerator.h"

DealGenerator::DealGenerator(QObject *parent) : QObject(parent) {
    initializeTemplates();
}

void DealGenerator::initializeTemplates() {
    // 100% Olive Oil Industry Specific Templates
    m_templates << DealTemplate{"Bulk Tunisian EVOO (April Batch)", "Tunis-Oil Coop", "MARKET LEAD", "https://www.tunisiatrading.tn", 
        "Premium extra virgin bulk supply. Certified low-acidity (0.2%) for 2026 export standards."};
    
    m_templates << DealTemplate{"Spanish Picual Future Contracts", "Andalusia Trade", "LOCK-IN RATE", "https://www.asoliva.com", 
        "Secure Q4 2026 delivery of high-yield Picual. Protection against projected Mediterranean drought pricing."};

    m_templates << DealTemplate{"Pieralisi Malaxation Efficiency Kit", "Pieralisi Group", "-20% UPGRADE", "https://www.pieralisi.com", 
        "Retrofit for existing EcoLine systems. Drastically reduces oxidation and improves sensory scores."};

    m_templates << DealTemplate{"Alfa Laval Sigma Decanter Shell", "Alfa Laval", "PRE-ORDER", "https://www.alfalaval.com", 
        "Advanced separation tech for high-throughput mills. 5% reduction in oil loss during extraction."};

    m_templates << DealTemplate{"Dark Green 'Dorica' Glass (750ml)", "The Olive Oil Source", "$0.58 / UNIT", "https://www.oliveoilsource.com", 
        "IOOC-standard UV protection glass. Essential for long-term polyphenol preservation in 2026."};

    m_templates << DealTemplate{"Nitrogen Sealing Bottling Line", "Inox-Fill Italy", "LEASE PROMO", "https://www.inox-meccanica.com", 
        "Complete automated line with nitrogen injection. Guarantees 24-month shelf life for premium oils."};

    m_templates << DealTemplate{"B2B Private Labeling (Tunisia)", "Sfax Packaging", "SAVE 15%", "https://www.newmapak.com", 
        "Turnkey bottling and labeling service for startups. Export-ready certification for US/EU markets."};

    m_templates << DealTemplate{"Organic Alperujo Soil Nutrition", "BioGro Solutions", "FREE TRIAL", "https://www.omri.org", 
        "Circular fertilizer made from olive press remnants. Boosts grove yields by 10% naturally."};

    m_templates << DealTemplate{"Pellenc Mobile Olive Shakers", "Pellenc Harvesting", "BUNDLE SAVINGS", "https://www.pellenc.com", 
        "The 2026 industry standard for damage-free harvesting. Preserves the fruit skin for higher quality oil."};

    m_templates << DealTemplate{"High-Phenol Certification Pack", "Industry Labs EU", "COMBO DEAL", "https://www.hannainst.com", 
        "Full lab testing suite for Health-Claim labeling. Get the 'High-Polyphenol' official 2026 seal."};

    m_templates << DealTemplate{"EVOO Export Logistics (EU-US)", "Mediterranean Freight", "FIXED QUOTE", "https://www.msc.com", 
        "Temperature-controlled shipping for premium batches. Door-to-door tracking for high-end retail."};

    m_templates << DealTemplate{"Stainless Steel Malaxer Tanks", "Mori-Tem Italy", "-10% INTRO", "https://www.mori-tem.it", 
        "Vertical malaxers for artisanal cold-press. Designed for high-polyphenol retention and rapid cleaning."};
}

QList<GeneratedDeal> DealGenerator::generateDeals(int count) {
    QList<GeneratedDeal> result;
    QList<int> usedIndices;
    
    int total = m_templates.size();
    if (total == 0) return result;

    for (int i = 0; i < count; ++i) {
        int idx = QRandomGenerator::global()->bounded(total);
        // Avoid duplicates in the same batch
        while (usedIndices.contains(idx) && usedIndices.size() < total) {
            idx = QRandomGenerator::global()->bounded(total);
        }
        usedIndices << idx;
        
        DealTemplate t = m_templates[idx];
        GeneratedDeal d;
        d.title = t.title;
        d.supplier = t.supplier;
        d.url = t.url;
        d.description = t.baseDesc;
        
        // Dynamic Variation: Add randomized "Flash" or "AI Detected" prefix
        int variant = QRandomGenerator::global()->bounded(5);
        if (variant == 0) d.savings = "AI DETECTED: " + t.category;
        else if (variant == 1) d.savings = "FLASH DEAL: " + t.category;
        else d.savings = t.category;

        result << d;
    }
    return result;
}
